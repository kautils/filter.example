#include <vector>
#include "flow.h"
#include <string>
#include <array>
#include <numeric>

constexpr int kLengthAdj = 2;

struct element{
    void* data;
    uint64_t size;
}__attribute__((aligned(8)));

struct example{
    std::vector<element> res_nu;
    std::vector<std::string*> nu_buffer;
    std::vector<double> res;
    std::vector<uint64_t> res_index;
    uint64_t len=0;
    inline static std::array<uint64_t,kLengthAdj> kV;
    std::array<uint64_t,kLengthAdj>::iterator v_cur;
    std::string state_id_buffer;
};

bool kInitOnce = [](){
    std::iota(example::kV.begin(),example::kV.end(),0);
    return true;
}.operator()();

struct filter_lookup_elem{
    const char * key = 0; 
    void * value = nullptr;
}__attribute__((aligned(sizeof(uintptr_t))));

#define m(object) reinterpret_cast<example*>(object->fm)



int fmain(filter * f);
void state_reset(filter * f);
bool state_next(filter * f);
const char * state_id(filter * f);
bool output_is_uniformed(filter * f);
uint64_t output_bytes(filter * f);
uint64_t output_size(filter * f);
void* output(filter * f);
uint64_t* index(filter * f);
const char* id(filter * f);
const char* id_hr(filter * f);
bool database_close_always(filter * f);


struct filter_lookup_table_example{
    filter_lookup_elem main{.key="fmain",.value=(void*)fmain};
    filter_lookup_elem output{.key="output",.value=(void*)::output};
    filter_lookup_elem output_size{.key="output_size",.value=(void*)::output_size};
    filter_lookup_elem output_bytes{.key="output_bytes",.value=(void*)::output_bytes};
    filter_lookup_elem index{.key="index",.value=(void*)::index};
    filter_lookup_elem id{.key="id",.value=(void*)::id};
    filter_lookup_elem id_hr{.key="id_hr",.value=(void*)::id_hr};
    filter_lookup_elem member{.key="member",.value=(void*)new example};
    filter_lookup_elem state_reset{.key="state_reset",.value=(void*)::state_reset};
    filter_lookup_elem state_next{.key="state_next",.value=(void*)::state_next};
    filter_lookup_elem state_id{.key="state_id",.value=(void*)::state_id};
    filter_lookup_elem output_is_uniformed{.key="output_is_uniformed",.value=(void*)::output_is_uniformed};
    filter_lookup_elem database_close_always{.key="database_close_always",.value=(void*)::database_close_always};
    filter_lookup_elem sentinel{.key=nullptr,.value=nullptr};
} __attribute__((aligned(sizeof(uintptr_t))));
    
#define UNIFORMED
#ifdef UNIFORMED

int fmain(filter * f) {
    auto arr = reinterpret_cast<double*>(f->input(f));
    auto len = f->input_bytes(f)/sizeof(double);

    m(f)->len=len;
    m(f)->res.resize(0);
    m(f)->res_index.resize(0);
    for(auto i = 0; i < len ; ++i){
        if(0==i%2){
            m(f)->res.push_back(arr[i]+123456789);
            m(f)->res_index.push_back(static_cast<uint64_t>(i));
        }
    }
    return 0;
}
bool output_is_uniformed(filter * f){ return true; }
uint64_t output_size(filter * f) { return m(f)->res.size(); }
uint64_t output_bytes(filter * f) { return m(f)->res.size()*sizeof(double); }
void* output(filter * f) { return m(f)->res.data(); }

#else

int fmain(filter * f) {
    auto arr = reinterpret_cast<double*>(f->input(f));
//    auto len = f->input_bytes(f)/sizeof(double);
    auto len = f->input_size(f);
    m(f)->len=len;
    m(f)->res_nu.resize(0);
    m(f)->res_index.resize(0);
    {
        for(auto & elem : m(f)->nu_buffer)delete elem;
        m(f)->nu_buffer.resize(0);
    }
    
    printf("%lld\n",len);fflush(stdout);
    for(auto i = 0; i < len; ++i){
        if(0==i%2){
            m(f)->res_index.push_back(static_cast<uint64_t>(i));
            m(f)->nu_buffer.push_back(new std::string{std::to_string(i)+"abcdefg"});
            printf("%s\n",(std::to_string(i)+"abcdefg").data()); fflush(stdout);
            m(f)->res_nu.push_back({.data=m(f)->nu_buffer.back()->data(),.size=m(f)->nu_buffer.back()->size()});
        }
    }
    return 0;
}
bool output_is_uniformed(filter * f){ return false; }
uint64_t output_size(filter * f) { return m(f)->res_nu.size(); }
uint64_t output_bytes(filter * f) { return sizeof(decltype(m(f)->res_nu)::value_type)*m(f)->res_nu.size(); }
void* output(filter * f) { return m(f)->res_nu.data(); }


#endif



//bool database_close_always(filter * f){ return true; }
bool database_close_always(filter * f){ return false; }
void state_reset(filter * f){ m(f)->v_cur=m(f)->kV.begin(); }
const char * state_id(filter * f){ return (m(f)->state_id_buffer=std::to_string(*m(f)->v_cur)).data(); }
bool state_next(filter * f){ return m(f)->kV.end() != ++m(f)->v_cur; };

uint64_t* index(filter * f){ return m(f)->res_index.data(); }
const char* id(filter * f) { return FILTER_ID; }
const char* id_hr(filter * f) { return FILTER_ID_HR; }


extern "C" uint64_t size_of_pointer(){ return sizeof(uint64_t); }
extern "C" filter_lookup_table * lookup_table_initialize(){ 
    return reinterpret_cast<filter_lookup_table*>(new filter_lookup_table_example);
}

extern "C" void lookup_table_free(filter_lookup_table * f){
    auto entity = reinterpret_cast<filter_lookup_table_example*>(f);

    {
        auto m = reinterpret_cast<example*>(entity->member.value);
        for(auto elem : m->nu_buffer)delete elem;
    }
    
    delete reinterpret_cast<example*>(entity->member.value);
    delete entity; 
}
