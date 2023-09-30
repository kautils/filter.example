#include <vector>
#include "flow.h"
#include <string>
#include <array>
#include <numeric>

constexpr int kLengthAdj = 100;

struct subtract{
    std::vector<double> res;
    uint64_t len=0;
    inline static std::array<uint64_t,100> kV;
    std::array<uint64_t,100>::iterator v_cur;
    std::string state_id_buffer;
};

bool kInit_once = [](){
    std::iota(subtract::kV.begin(),subtract::kV.end(),0);
    return true;
}.operator()();

struct filter_lookup_elem{
    const char * key = 0; 
    void * value = nullptr;
}__attribute__((aligned(sizeof(uintptr_t))));

#define m(object) reinterpret_cast<subtract*>(object->m)



int fmain(filter * f);
void state_reset(filter * f);
bool state_next(filter * f);
const char * state_id(filter * f);
uint64_t output_bytes(filter * f);
void* output(filter * f);
const char* id(filter * f);
const char* id_hr(filter * f);


struct filter_lookup_table_subtract{
    filter_lookup_elem main{.key="fmain",.value=(void*)fmain};
    filter_lookup_elem output{.key="output",.value=(void*)::output};
    filter_lookup_elem output_bytes{.key="output_bytes",.value=(void*)::output_bytes};
    filter_lookup_elem id{.key="id",.value=(void*)::id};
    filter_lookup_elem id_hr{.key="id_hr",.value=(void*)::id_hr};
    filter_lookup_elem member{.key="member",.value=(void*)new subtract};
    filter_lookup_elem state_reset{.key="state_reset",.value=(void*)::state_reset};
    filter_lookup_elem state_next{.key="state_next",.value=(void*)::state_next};
    filter_lookup_elem state_id{.key="state_id",.value=(void*)::state_id};
    filter_lookup_elem sentinel{.key=nullptr,.value=nullptr};
} __attribute__((aligned(sizeof(uintptr_t))));
    

int fmain(filter * f) {
    auto arr = reinterpret_cast<double*>(f->input(f));
    auto len = f->input_bytes(f)/sizeof(double);
    
    auto a = m(f)->len;
    m(f)->res.push_back(100);
    
    if(m(f)->res.size() < len) m(f)->res.resize(len);
    m(f)->len=len;
    for(auto i = 0; i < len ; i++){
        m(f)->res[i] = arr[i]+123456789;
    }
    return 0;
}


void state_reset(filter * f){ m(f)->v_cur=m(f)->kV.begin(); }
const char * state_id(filter * f){ return (m(f)->state_id_buffer=std::to_string(*m(f)->v_cur)).data(); }
bool state_next(filter * f){ 
    return m(f)->kV.end() != ++m(f)->v_cur; 
};


uint64_t output_bytes(filter * f) { return m(f)->res.size()*sizeof(double); }
void* output(filter * f) { return m(f)->res.data(); }
const char* id(filter * f) { return FILTER_ID; }
const char* id_hr(filter * f) { return FILTER_ID_HR; }


extern "C" uint64_t size_of_pointer(){ return sizeof(uint64_t); }
extern "C" filter_lookup_table * lookup_table_initialize(){ 
    return reinterpret_cast<filter_lookup_table*>(new filter_lookup_table_subtract);
}

extern "C" void lookup_table_free(filter_lookup_table * f){
    auto entity = reinterpret_cast<filter_lookup_table_subtract*>(f);
    delete reinterpret_cast<subtract*>(entity->member.value);
    delete entity; 
}
