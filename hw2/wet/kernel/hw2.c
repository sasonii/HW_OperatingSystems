#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/errno.h>

asmlinkage long sys_hello(void) {
	printk("Hello, World!\n");
	return 0;
}

asmlinkage long sys_set_weight(int weight){
  if(weight < 0){
    return -EINVAL;
  }
  current->weight = weight;
  return 0;
}

asmlinkage long sys_get_weight(void){  
  return current->weight;  
}

asmlinkage long sys_get_ancestor_sum(void) {
    struct task_struct *current_task = current;
    long sum = 0;
	
	// while not init
    while (current_task->pid != 1) {
        sum += current_task->weight;
        current_task = current_task->real_parent;
    }
    // add init sum
    sum += current_task->weight;

    return sum;
}

static struct task_struct *find_heaviest_descendant(struct task_struct *task, int father) {
	struct task_struct *heaviest_descendant;
	int heaviest_weight;
	
	if(father == 1){
		heaviest_descendant = NULL;
		heaviest_weight = 0;
	} else {
		heaviest_descendant = task;
		heaviest_weight = task->weight;
	}
	
    struct task_struct *child;
    if (!list_empty(&(task->children))){    
		list_for_each_entry(child, &(task->children), sibling) {
			struct task_struct *descendant = find_heaviest_descendant(child, 0);
			if (descendant != NULL && descendant->weight > heaviest_weight) {
				heaviest_weight = descendant->weight;				
				heaviest_descendant = descendant;
			} else if (descendant != NULL && descendant->weight == heaviest_weight && descendant->pid < heaviest_descendant->pid) {
            // If weights are equal, choose the one with the smallest pid
            heaviest_descendant = descendant;
			}
		}
	}

	return heaviest_descendant;
}

asmlinkage long sys_get_heaviest_descendant(void) {
	struct task_struct *heaviest_descendant;
    struct task_struct *current_task;
    
    current_task = current;

    if (list_empty(&(current_task->children)))
        return -ECHILD;
	
     heaviest_descendant= find_heaviest_descendant(current_task, 1);

    return heaviest_descendant->pid;
}
