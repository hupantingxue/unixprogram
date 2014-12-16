#include <Ice/Ice.h>
#include <Printer.h>

using namespace std;
using namespace PrintDemo;

int main(int argc, char *argv[]) {
    int status = 0;
    Ice::CommunicatorPtr ic;

    try {
        ic = Ice::initialize(argc, argv);
        Ice::ObjectPrx base 
            = ic->stringToProxy(
                "SimplePrinter:default -p 10000");
        PrinterPrx printer = PrinterPrx::checkedCast(base);
        if (!printer)
            throw "Invalid proxy";

        printer->printString("Hello world!");
    } catch (const Ice::Exception &e) {
        cerr << e << endl;
        status = 1;
    } catch (const char *msg) {
        cerr << msg << endl;
        status = 1;
    }

    if (ic) {
        ic->destroy();
    }
    return status;
}
