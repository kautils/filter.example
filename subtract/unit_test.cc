struct filter_handler{
    
};

struct filter{
    const char * filer_main_k="filer_main";
    int (*filer_main_v)(filter_handler * hdl);
} __attribute__((aligned(8)));

extern "C" int filter_main(filter_handler * hdl) {
    return 0;
}

filter * filter_initialize(){ 
    auto res= new filter{}; 
    res->filer_main_v = filter_main;
    return res;
}
void filter_free(filter * f){ delete f; }



#include <stdio.h>

int main(){
    
    
    auto fhdl = filter_handler{};
    auto f = filter_initialize();
    auto btree = reinterpret_cast<const char **>(f);
    
    printf("%s\n",btree[0]);
    
    
    
    
    return 0;
}