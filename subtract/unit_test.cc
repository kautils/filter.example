
#ifdef TMAIN_KAUTIL_FILTER_ALTHMETIC_SUBTRACT_SHARED

#include <vector>
#include <string>
#include "kautil/cache/virtual_file/virtual_file.h"
#include "flow.h"


struct subtract{
    std::vector<double> res;
    uint64_t len=0;
};

#define m(object) reinterpret_cast<subtract*>(object->m)

struct filter_lookup_table_subtract{
    filter_lookup_elem main{.key="fmain"};
    filter_lookup_elem output{.key="output"};
    filter_lookup_elem output_bytes{.key="output_bytes"};
    filter_lookup_elem database_type{.key="database_type"};
    filter_lookup_elem id{.key="id"};
    filter_lookup_elem id_hr{.key="id_hr"};
    filter_lookup_elem member{.key="member"};
    filter_lookup_elem sentinel{.key=nullptr,.value=nullptr};
} __attribute__((aligned(8)));

extern "C" int fmain(filter * f) {
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




extern "C" uint64_t output_bytes(filter * f) { return m(f)->res.size()*sizeof(double); }
extern "C" void* output(filter * f) { return m(f)->res.data(); }
extern "C" const char* id(filter * f) { return FILTER_ID; }
extern "C" const char* id_hr(filter * f) { return FILTER_ID_HR; }
extern "C" int database_type(filter * f){ return FILTER_DATABASE_TYPE_SQLITE3; }


filter_lookup_table * lookup_tb_initialize(){ 
    auto res= new filter_lookup_table_subtract{}; 
    res->main.value = (void*)fmain;
    res->output.value = (void*)output;
    res->output_bytes.value=(void*)output_bytes;
    res->id.value=(void*)id;
    res->id_hr.value=(void*)id_hr;
    res->member.value = new subtract{};
    res->database_type.value = (void *) database_type;
    return reinterpret_cast<filter_lookup_table*>(res);
}

void lookup_tb_free(filter_lookup_table * f){
    auto m = reinterpret_cast<filter_lookup_table_subtract*>(f)->member;
    delete reinterpret_cast<subtract*>(m.value);
    delete f; 
}

int main(){
    
    remove("R:\\flow\\build\\android\\filter.arithmetic.subtract\\KautilFilterArithmeticSubtract.0.0.1\\4724af5.sqlite");
    
    auto input_len = 100; // all the input/output inside a chain is the same,if the result structure are counted as one data 
    auto arr = new double[input_len];
    for(auto i = 0; i < input_len; ++i)arr[i] = i;
    
    auto fhdl = filter_handler_initialize();
    filter_handler_set_io_length(fhdl,input_len);
    filter_handler_set_local_uri(fhdl,"./");

    {
        filter_handler_input(fhdl,arr,sizeof(double));
        filter_handler_push_with_lookup_table(fhdl,lookup_tb_initialize,lookup_tb_free);
        filter_handler_execute(fhdl);
    }
    
    filter_handler_free(fhdl);
    delete [] (double*) arr;


    return 0;
}

#endif