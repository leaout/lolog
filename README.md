# lolog
Write log without lock.
## Build & Install
```bash
./build.sh debug/release
make insall
```
## Example
```c++
#include "Lolog.h"
using namespace lolog;

int main(int argc, char* argv[]){
    lolog::init_logging(argv[0],5,200<<20);
    lolog::set_log_level("Debug");

    debug("this is debug");
    DEBUGEX("this is debug");
    LODEBUG() <<"this is macro debug";

    info("this is info");
    INFOEX("this is info");
    LOINFO() <<"this is macro info";

    warn("this is warn");
    WARNEX("this is warn");
    LOWARN() <<"this is macro warn";

    error("this is error");
    ERROREX("this is error");
    LOERROR()<< "this is macro error";

    fatal("this is fatal");
    FATALEX("this is fatal");
    LOFATAL() << "this is macro fatal";
    return 0;
}

```