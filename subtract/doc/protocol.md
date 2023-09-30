
* lookup table :
  * element size : sizeof(uintptr_t)*2
  * nullend
```c++
struct some_table_element{
    const char * key;
    void * value;
} __attribute__((aligned(sizeof(uintptr_t))));
struct some_table{
    some_table_element func0={.key="func0",.value=(void*)pointer_of_func0};
    some_table_element func1={.key="func1",.value=(void*)pointer_of_func1};
    ...
    some_table_element funcN={.key="funcN",.value=(void*)pointer_of_funcN};
    some_table_element sentinel{.key=0,value=0};
};
```