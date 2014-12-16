import sys, traceback, Ice
#from gen.python import PrintDemo
import PrintDemo

status = 0
ic = None
try:
    ic = Ice.initialize(sys.argv)
    base = ic.stringToProxy("SimplePrinter:default -p 10000")
    printer = PrintDemo.PrinterPrx.checkedCast(base)

    if not printer:
        raise RuntimeError("Invalid proxy")
    printer.printString("Hello World from hupantingxue!")
except:
    traceback.print_exc()
    status = 1

if ic:
    # Clean up
    try:
        ic.destroy()
    except:
        traceback.print_exc()
        status = 1
sys.exit(status)
