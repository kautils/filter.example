
#ifdef TMAIN_KAUTIL_FILTER_ALTHMETIC_SUBTRACT_SHARED

#include <stdio.h>
#include <cstring>
#include "kautil/algorithm/btree_search/btree_search.h"
#include <algorithm>

struct filter;
struct filter_handler{ 
    filter * previous=0; 
    filter * last=0; 
};

struct filter{
    int (*main)(filter * m,filter_handler * hdl);
    void * (*output)(filter *)=0;
    uint64_t(*output_bytes)(filter *)=0;
    void * (*input)(filter *)=0;
    uint64_t(*input_bytes)(filter *)=0;
    void* m=0;
    filter_handler * hdl=0;
    
} __attribute__((aligned(8)));

///@note input is common
void* filter_input(filter * f) { return f->hdl->last->output(f->hdl->last); }
uint64_t filter_input_bytes(filter * f) { 
    return f->hdl->previous->output_bytes(f->hdl->previous); 
}


struct filter_lookup_table;
struct filter_lookup_elem{
    const char * key = 0; 
    void * value = nullptr;
}__attribute__((aligned(8)));

void * filter_lookup(filter_lookup_table * flookup_table,const char * key){
    // sorting is bad for this process. i neither want  not to change the order of the flookup_table nor to copy it 
    auto arr = reinterpret_cast<char **>(flookup_table);
    for(;;arr+=sizeof(filter_lookup_elem)/sizeof(uintptr_t)){
        if(!strcmp(key,arr[0]))return reinterpret_cast<void*>(arr[1]);
        if(nullptr==arr[0]) break;
    }
    return nullptr;
}



struct subtract_o{}__attribute__((aligned(8)));
struct subtract_i{}__attribute__((aligned(8)));

struct subtract{
    subtract_o o;
    subtract_o i;
};

#define m(object) reinterpret_cast<subtract*>(object->m)

struct filter_lookup_table{
    filter_lookup_elem main{.key="fmain"};
    filter_lookup_elem output{.key="output"};
    filter_lookup_elem output_bytes{.key="output_bytes"};
    filter_lookup_elem input{.key="input"};
    filter_lookup_elem input_bytes{.key="input_bytes"};
    filter_lookup_elem member{.key="member"};
    filter_lookup_elem sentinel{.key=nullptr,.value=nullptr};
} __attribute__((aligned(8)));

extern "C" int fmain(filter * m,filter_handler * hdl) {
    hdl->previous = hdl->last; 
    hdl->last = m;
    printf("AAA\n");
    return 0;
}

extern "C" uint64_t output_bytes(filter * f) { return sizeof(subtract_o); }
extern "C" void* output(filter * f) { return &m(f)->o; }


filter_lookup_table * filter_lookup_table_initialize(){ 
    auto res= new filter_lookup_table{}; 
    res->main.value = (void*)fmain;
    res->output.value = (void*)output;
    res->output_bytes.value=(void*)output_bytes;
    res->member.value = new subtract{};
    return res;
}

void filter_lookup_table_free(filter_lookup_table * f){
    delete reinterpret_cast<subtract*>(f->member.value);
    delete f; 
}

int cnt = 0;
bool __mock_if(bool cond) {
    // assume this return 0/1. possible to express with small code 
    printf("AAA\n");
    return cnt++;
}




int main(){
    
    
#define if(abc) if(__mock_if(abc))
    
    // cycle to allocate memocy. i doubt this is not optimized completely by gcc/clang.
    // below may make compiler confuse because of mock if. but mock if is important for this program. 
    {
        int abc = 0;
        for(auto i = 0; i < 2; ++i){
            if(!abc){ // mock if
                printf("AAAAAAA\n");
            }else{
                printf("BBBBBBB\n");
            }
        }
    }
#undef if

    {
        // cycle to run. i think this should be purely optimized by gcc/clang
        int abc = 0;
        for(auto i = 0; i < 2; ++i){
            if(!abc){ // pure(ordinal) if
                printf("AAAAAAA\n");
            }else{
                printf("BBBBBBB\n");
            }
        }
    }
    
    
    
//    struct{
//        double a = 123.4;
//        double b = 567.8;
//    }input __attribute__((aligned(8)));
//    
//    filter inputer;
//
//    
//    using output_t = void*(*)(filter *);
//    using input_t = void*(*)(filter *);
//    using output_bytes_t = uint64_t(*)(filter *);
//    using input_bytes_t = uint64_t(*)(filter *);
//    
//    auto f = filter{};
//    auto fhdl = filter_handler{};
//    auto flookup = filter_lookup_table_initialize();
//    f.main= (decltype(f.main))filter_lookup(flookup,"fmain");
//    f.output= (output_t)filter_lookup(flookup,"output");
//    f.output_bytes= (output_bytes_t)filter_lookup(flookup,"output_bytes");
//    f.input= (input_t)filter_lookup(flookup,"input");
//    f.input= (input_t) filter_input;
//    f.input_bytes= (input_bytes_t) filter_input_bytes;
//    f.hdl=&fhdl;
//    
//    
//    if(f.main){
//        f.main(&f,&fhdl);
//    }
//    if(f.output_bytes){
//        printf("%lld\n",f.output_bytes(&f));
//    }
//    if(f.input_bytes){
//        
//        f.input_bytes(&f);
//        printf("%lld\n",f.input_bytes(&f));
//    }
//    
//    // 
//    filter_lookup_table_free(flookup);
    
    
    return 0;
}

#endif