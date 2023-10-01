#include <vector>
#include "flow.h"
#include <string>
#include <array>
#include <numeric>

constexpr int kLengthAdj = 2;

struct subtract{
    std::vector<double> res;
    uint64_t len=0;
    inline static std::array<uint64_t,kLengthAdj> kV;
    std::array<uint64_t,kLengthAdj>::iterator v_cur;
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

#define m(object) reinterpret_cast<subtract*>(object->fm)



int fmain(filter * f);
void state_reset(filter * f);
bool state_next(filter * f);
const char * state_id(filter * f);
bool is_uniformed(filter * f);
uint64_t output_bytes(filter * f);
uint64_t output_size(filter * f);
void* output(filter * f);
const char* id(filter * f);
const char* id_hr(filter * f);
bool database_close_always(filter * f);


struct filter_lookup_table_subtract{
    filter_lookup_elem main{.key="fmain",.value=(void*)fmain};
    filter_lookup_elem output{.key="output",.value=(void*)::output};
    filter_lookup_elem output_size{.key="output_size",.value=(void*)::output_size};
    filter_lookup_elem output_bytes{.key="output_bytes",.value=(void*)::output_bytes};
    filter_lookup_elem id{.key="id",.value=(void*)::id};
    filter_lookup_elem id_hr{.key="id_hr",.value=(void*)::id_hr};
    filter_lookup_elem member{.key="member",.value=(void*)new subtract};
    filter_lookup_elem state_reset{.key="state_reset",.value=(void*)::state_reset};
    filter_lookup_elem state_next{.key="state_next",.value=(void*)::state_next};
    filter_lookup_elem state_id{.key="state_id",.value=(void*)::state_id};
    filter_lookup_elem is_uniformed{.key="is_uniformed",.value=(void*)::is_uniformed};
    filter_lookup_elem database_close_always{.key="database_close_always",.value=(void*)::database_close_always};
    filter_lookup_elem sentinel{.key=nullptr,.value=nullptr};
} __attribute__((aligned(sizeof(uintptr_t))));
    

int fmain(filter * f) {
    auto arr = reinterpret_cast<double*>(f->input(f));
    auto len = f->input_bytes(f)/sizeof(double);
    
    if(m(f)->res.size() < len) m(f)->res.resize(len);
    m(f)->len=len;
    for(auto i = 0; i < len ; i++){
        printf("%llf\n",arr[i]);fflush(stdout);
        m(f)->res[i] = arr[i]+123456789;
    }
    return 0;
}



bool database_close_always(filter * f){ return true; }
bool is_uniformed(filter * f){ return true; }
void state_reset(filter * f){ m(f)->v_cur=m(f)->kV.begin(); }
const char * state_id(filter * f){ return (m(f)->state_id_buffer=std::to_string(*m(f)->v_cur)).data(); }
bool state_next(filter * f){ 
    return m(f)->kV.end() != ++m(f)->v_cur; 
};


uint64_t output_size(filter * f) { return m(f)->res.size(); }
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
