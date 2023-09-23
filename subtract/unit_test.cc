
#ifdef TMAIN_KAUTIL_FILTER_ALTHMETIC_SUBTRACT_SHARED

#include <stdio.h>
#include <cstring>
#include "kautil/algorithm/btree_search/btree_search.h"
#include <algorithm>

struct filter;
struct filter_handler{ filter * last=0; };
struct filter{
    int (*main)(filter * m,filter_handler * hdl);
    void * (*output)()=0;
    uint64_t(*output_bytes)()=0;
    void * (*input)()=0;
    uint64_t(*input_bytes)()=0;
} __attribute__((aligned(8)));

struct filter_lookup_table;
struct filter_lookup_elem{
    const char * key = 0; 
    void * value = nullptr;
}__attribute__((aligned(8)));

void * filter_lookup(filter_lookup_table * flookup_table,const char * key){
    auto arr = reinterpret_cast<char **>(flookup_table);
    for(;;arr+=sizeof(filter_lookup_elem)/sizeof(uintptr_t)){
        if(!strcmp(key,arr[0]))return reinterpret_cast<void*>(arr[1]);
        if(nullptr==arr[0]) break;
    }
    return nullptr;
}


struct subtract_o{};
struct subtract_i{};


struct filter_lookup_table{
    filter_lookup_elem main{.key="fmain"};
    filter_lookup_elem output{.key="output"};
    filter_lookup_elem aaa{.key="aaa"};
    filter_lookup_elem output_bytes{.key="output_bytes"};
    filter_lookup_elem input{.key="input"};
    filter_lookup_elem input_bytes{.key="input_bytes"};
    filter_lookup_elem sentinel{.key=nullptr,.value=nullptr};
} __attribute__((aligned(8)));

extern "C" int fmain(filter * m,filter_handler * hdl) {
    hdl->last = m;
    printf("AAA\n");
    return 0;
}

extern "C" uint64_t output_bytes() { return sizeof(subtract_o); }
extern "C" uint64_t input_bytes() { return sizeof(subtract_i); }



filter_lookup_table * filter_lookup_table_initialize(){ 
    auto res= new filter_lookup_table{}; 
    res->main.value = (void*)fmain;
    res->output.value = new subtract_o;
    res->input.value = new subtract_i;
    res->output_bytes.value=(void*)output_bytes;
    res->input_bytes.value=(void*)input_bytes;
    
    return res;
}

void filter_lookup_table_free(filter_lookup_table * f){ 
    delete reinterpret_cast<subtract_o*>(f->output.value);
    delete reinterpret_cast<subtract_i*>(f->input.value);
    delete f; 
}




int main(){
    
    using output_t = void*(*)();
    using input_t = void*(*)();
    using output_bytes_t = uint64_t(*)();
    using input_bytes_t = uint64_t(*)();
    
    auto f = filter{};
    auto fhdl = filter_handler{};
    auto flookup = filter_lookup_table_initialize();
    f.main= (decltype(f.main))filter_lookup(flookup,"fmain");
    f.output= (output_t)filter_lookup(flookup,"output");
    f.output_bytes= (output_bytes_t)filter_lookup(flookup,"output_bytes");
    f.input= (input_t)filter_lookup(flookup,"input");
    f.input_bytes= (input_bytes_t) filter_lookup(flookup,"input_bytes");
    
    
    if(f.main){
        f.main(&f,&fhdl);
    }
    if(f.output_bytes){
        printf("%lld\n",f.output_bytes());
    }
    if(f.input_bytes){
        printf("%lld\n",f.input_bytes());
    }
    
    // 
    filter_lookup_table_free(flookup);
    
    
    return 0;
}

#endif