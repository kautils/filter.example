
#ifdef TMAIN_KAUTIL_FILTER_ALTHMETIC_SUBTRACT_SHARED

#include <stdio.h>
#include <cstring>
#include "kautil/algorithm/btree_search/btree_search.h"
#include <algorithm>

struct filter;
struct filter_handler{ filter * last=0; };
struct filter{
    int (*main)(filter * m,filter_handler * hdl);
} __attribute__((aligned(8)));

struct filter_lookup_table;
void * filter_lookup(filter_lookup_table * flookup_table,const char * key){
    
    struct Dum{ char* dum[2]; };
    auto beg = reinterpret_cast<Dum*>(flookup_table);
    auto buf = reinterpret_cast<Dum*>(flookup_table);
    while(buf->dum[0]) ++buf;
    auto len = (buf-beg);
    auto sortf = [](Dum const& l, Dum const& r){ return !strcmp(l.dum[0],r.dum[0]); };
    if(!std::is_sorted(beg,beg+len,sortf)) std::sort(beg,beg+len,sortf);
    
    auto fp = (char**)0;
    auto btree = reinterpret_cast<char**>(flookup_table);
    if(0==kautil_bt_search_v3((void ** )&fp,(const void*)&key,btree,btree+len,sizeof(char*)*2, nullptr,[](auto l,auto r,auto arg){
        return strcmp(*(const char **)l,*(const char **)r);
    })){
        return reinterpret_cast<void*>(fp[1]);
    }
    return nullptr;
}


struct subtract_o{};
struct subtract_i{};
struct filter_lookup_elem{
    const char * key = 0; 
    void * value = nullptr;
};


struct filter_lookup_table{
    filter_lookup_elem main{.key="fmain"};
    filter_lookup_elem output{.key="output"};
    filter_lookup_elem input{.key="input"};
    filter_lookup_elem sentinel{.key=nullptr,.value=nullptr};
} __attribute__((aligned(8)));


extern "C" int fmain(filter * m,filter_handler * hdl) {
    hdl->last = m;
    printf("AAA");
    return 0;
}

filter_lookup_table * filter_lookup_table_initialize(){ 
    auto res= new filter_lookup_table{}; 
    res->main.value = (void*)fmain;
    res->output.value = new subtract_o;
    res->input.value = new subtract_i;
    return res;
}

void filter_lookup_table_free(filter_lookup_table * f){ 
    delete reinterpret_cast<subtract_o*>(f->output.value);
    delete reinterpret_cast<subtract_i*>(f->input.value);
    delete f; 
}




int main(){
    
    auto f = filter{};
    auto fhdl = filter_handler{};
    auto flookup = filter_lookup_table_initialize();
    f.main= (decltype(f.main))filter_lookup(flookup,"fmain");
    if(f.main){
        f.main(&f,&fhdl);
    }
    filter_lookup_table_free(flookup);
    
    
    return 0;
}

#endif