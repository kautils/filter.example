
#ifdef TMAIN_KAUTIL_FILTER_ALTHMETIC_SUBTRACT_SHARED

#include <stdio.h>

struct filter_handler{
    
};

struct filter{
    const char * filer_main_k="filter_main";
    int (*filer_main_v)(filter_handler * hdl);
} __attribute__((aligned(8)));

extern "C" int filter_main(filter_handler * hdl) {
    printf("AAA");
    return 0;
}

filter * filter_initialize(){ 
    auto res= new filter{}; 
    res->filer_main_v = filter_main;
    return res;
}
void filter_free(filter * f){ delete f; }




#include <cstring>
#include "kautil/algorithm/btree_search/btree_search.h"


int main(){
    auto fhdl = filter_handler{};
    auto f = filter_initialize();
    auto btree = reinterpret_cast<const char **>(f);
    printf("%s\n",btree[0]);

    auto key = "filter_main";
    auto fp = (char**)0;

    auto len = sizeof(*f)/sizeof(uintptr_t);
    if(0==kautil_bt_search_v3((void ** )&fp,(const void*)&key,btree,btree+sizeof(*f)/sizeof(uintptr_t),sizeof(uintptr_t)*2, nullptr,[](auto l,auto r,auto arg){
        return strcmp(*(const char **)l,*(const char **)r);
    })){
        printf("+++%llx\n",(uintptr_t)*fp);
        reinterpret_cast<int(*)(filter_handler*)>(fp[1])(&fhdl);
    }else{
        printf("fail.");
    }
    
    
    
    
    return 0;
}

#endif