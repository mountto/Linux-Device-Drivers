//  xarray.c
//
#include <linux/xarray.h>
#include <linux/module.h>
#include <linux/slab.h>
 
static DEFINE_XARRAY(array);

static void xa_user_dump(const struct xarray *xa)
{
	void *entry = xa->xa_head;
	//unsigned int shift = 0; 
	printk("xarray: %px head %px flags %x marks %d %d %d\n", xa, entry,
		xa->xa_flags, xa_marked(xa, XA_MARK_0),
		xa_marked(xa, XA_MARK_1), xa_marked(xa, XA_MARK_2));


}
 
void xarray_test(void)
{
	unsigned long i;
	void *ret;
	struct item {
		unsigned long index;
		unsigned int order;
	};
	struct item *item;
	
	printk("xarray_user_test() starting...\n");
	item = kmalloc(sizeof(*item), GFP_KERNEL);
	item->index = 0;
	item->order = 100;
	printk("[0] item=%p\n", item);

	ret = xa_store(&array, 0, (void *)item, GFP_KERNEL); 
	for (i = 1; i <= 10; i += 2) { 
		item = kmalloc(sizeof(*item), GFP_KERNEL);
		printk("[%ld] item=%p, ", i, item);
		item->index = i;
		item->order = i + 100;
		ret = xa_store(&array, i, (void *)item, GFP_KERNEL); 
		printk("ret=%p\n", ret); 
		xa_user_dump(&array);
	}	
	//    rcu_read_lock();
	ret = xa_load(&array, 6);
	if(ret != NULL){
	printk("load ret=%p, %ld, %d\n",
		ret, ((struct item *)ret)->index, ((struct item *)ret)->order);
	}else {
	  printk("Khong load duoc\n");
	}
	//    rcu_read_unlock();
	// VD : 1
	// ví dụ ta xa_store() một entry mới (void*)0x44 vào index i=5 đã có, thì
	i = 5;
	ret = xa_store(&array,i, (void*)0x44,GFP_KERNEL);
	if(ret != NULL){
		//return cho ta một entry củ trước đó, rồi kernel sẻ cấp cho ta một entry mới
		printk("ret=%p\n", ret); 
	} else {
		printk("khong xa_store() duoc\n");
	}
	// VD : 2
	i = 2;
	ret = xa_store(&array,i, xa_load(&array,3),GFP_KERNEL);
	if(ret != NULL){
		//
		printk("ret=%p\n", ret); 
	} else {
		printk(" xa_store(&array,i, xa_load(&array,3),GFP_KERNEL) duoc\n");
		printk("ret=%p\n", ret); 

	}
		// VD : 3
	i = 9;
	ret = xa_store(&array,i, xa_load(&array,3),GFP_KERNEL);
	if(ret != NULL){
		//
		printk("ret=%p\n", ret); 
	} else {
		printk(" xa_store(&array,i, xa_load(&array,3),GFP_KERNEL) duoc\n");
		printk("ret=%p\n", ret); 

	}
		// VD : 4
	// ví dụ ta xa_store() một entry mới (void*)0x45 vào index mới, thì
	i = 6;
	ret = xa_store(&array,i, (void*)0x45,GFP_KERNEL);
	if(ret != NULL){
		//return cho ta một entry củ trước đó, rồi kernel sẻ cấp cho ta một entry mới
		printk("ret=%p\n", ret); 
	} else {
		printk("khong xa_store() duoc\n");
				printk("ret=%p\n", ret); 

	}

	//Kết luận:
	//Các index khác nhau có thể có cùng một entry (multi-index entry/mục nhập nhiều chỉ mục)



	//rcu_read_lock();
	ret = xa_load(&array, 8);
	if(ret != 0){
	printk("load ret=%p, %ld, %d\n",
		ret, ((struct item *)ret)->index, ((struct item *)ret)->order);
	}else {
	  printk("Khong load duoc\n");
	}
	//rcu_read_unlock();
	
	//    rcu_read_lock();
	ret = xa_erase(&array, 4);
	if(ret != 0){
	printk("erase ret=%p, %ld, %d\n",
		ret, ((struct item *)ret)->index, ((struct item *)ret)->order);
	}else {
	  printk("Khong erase duoc, ko ti`m tha'y\n");
	}
	//rcu_read_unlock();


	//rcu_read_lock();
	ret = xa_erase(&array, 7);
	if(ret != 0){
	printk("erase ret=%p, %ld, %d\n",
	       ret, ((struct item *)ret)->index, ((struct item *)ret)->order);
	}else {
	  printk("Khong erase duoc index,vi` index ko co'\n");
	}
	
	//rcu_read_unlock();

	i = 3;
	ret = xa_load(&array,i);
	if(ret != 0){
	ret = xa_find(&array, &i, ULONG_MAX, XA_PRESENT); //ti`m a->z, ne'u ko ti`m tha'y index do', thi` no' return index cuo'i cu`ng
	//vi` the' ba.n phai xa_load() xem co' index do' ko, ro`i ti`m = xa_find()
	
	printk("find ret=%p, %ld, %d\n",
		ret, ((struct item *)ret)->index, ((struct item *)ret)->order);
	}else {
	  printk("khong tim thay \n");
	}

	//Dump xarray
	printk("\t===Dump xarray===\n");
	/*
	xa_for_each(&array, i, ret) { 
		printk("each ret=%p, %ld, %d\n",
			ret, ((struct item *)ret)->index, ((struct item *)ret)->order);
	}
	*/	
		xa_for_each(&array, i, ret) { 
		printk("each ret=%p, %ld\n",
			ret, i);
	}	
 
	xa_destroy(&array); 
	printk("xarray_test() end.\n");
}

static int __init xarray_mod_init(void)
{
	printk("%s, Entering module\n", __func__);
	xarray_test();

	return 0;
}

static void __exit xarray_mod_exit(void)
{
	printk("%s, Exiting module\n", __func__);
}

module_init(xarray_mod_init);
module_exit(xarray_mod_exit);
MODULE_LICENSE("GPL");
