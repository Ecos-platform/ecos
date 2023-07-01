namespace cpp proxyfmu.thrift

include "defs.thrift"

service FmuService {

    void instantiate()

    defs.Status setup_experiment(1: double start, 2: double stop, 3: double tolerance)
    defs.Status enter_initialization_mode()
    defs.Status exit_initialization_mode()
    
    defs.Status step(1: double currentTime, 2: double stepSize)
    defs.Status terminate()
    defs.Status reset()
    void freeInstance()

    defs.IntegerRead read_integer(1: defs.ValueReferences vr)
    defs.RealRead read_real(1: defs.ValueReferences vr)
    defs.StringRead read_string(1: defs.ValueReferences vr)
    defs.BooleanRead read_boolean(1: defs.ValueReferences vr)

    defs.Status write_integer(1: defs.ValueReferences vr, 2: defs.IntArray value)
    defs.Status write_real(1: defs.ValueReferences vr, 2: defs.RealArray value)
    defs.Status write_string(1: defs.ValueReferences vr, 2: defs.StringArray value)
    defs.Status write_boolean(1: defs.ValueReferences vr, 2: defs.BooleanArray value)

}
