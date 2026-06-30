#include "test_helper.h"

#include <iostream>

void run_path_matcher_tests();
void run_http_parser_tests();
void run_router_tests();
void run_url_decoder_tests();
void run_http_serializer_tests();
void run_static_file_server_tests();
void run_middleware_tests();

int main() {
    std::cout << "\n";

    run_path_matcher_tests();
    run_http_parser_tests();
    run_router_tests();
    run_url_decoder_tests();
    run_http_serializer_tests();
    run_static_file_server_tests();
    run_middleware_tests();

    stats();
    std::cout << "\n";
    
    return 0;
}