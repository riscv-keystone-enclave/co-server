#include <array>
#include <thread>

#include "co_context/net.hpp"

extern void
enclave_main(int argc, char** argv);
extern void
httpd_main();



int
main(int argc, char** argv) {
    std::thread enclave_thread(enclave_main, argc, argv);

    httpd_main();

    enclave_thread.join();
    return 0;
}
