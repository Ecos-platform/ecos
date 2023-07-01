namespace cpp proxyfmu.thrift

typedef string FmuId
typedef string InstanceId
typedef i64 ValueReference
typedef i64 FmuState
typedef list<double> DirectionalDerivative
typedef list<ValueReference> ValueReferences
typedef list<i32> IntArray
typedef list<double> RealArray
typedef list<string> StringArray
typedef list<bool> BooleanArray

enum Status {
    OK_STATUS = 0,
    WARNING_STATUS = 1,
    DISCARD_STATUS = 2,
    ERROR_STATUS = 3,
    FATAL_STATUS = 4,
    PENDING_STATUS = 5
}

struct IntegerAttribute {
    1: optional i32 start,
}

struct RealAttribute {
    1: optional double start
}

struct StringAttribute {
    1: optional string start
}

struct BooleanAttribute {
    1: optional bool start
}

union ScalarVariableAttribute {
    1: IntegerAttribute integer_attribute,
    2: RealAttribute real_attribute,
    3: StringAttribute string_attribute,
    4: BooleanAttribute boolean_attribute
}

struct ScalarVariable {
    1: string name,
    2: ValueReference value_reference,
    4: optional string initial,
    5: optional string causality,
    6: optional string variability,
    7: ScalarVariableAttribute attribute
}

typedef list<ScalarVariable> ModelVariables

struct DefaultExperiment {
    1: double startTime,
    2: double stopTime,
    3: double tolerance,
    4: double stepSize
}

struct IntegerRead {
    1: list<i32> value,
    2: Status status
}

struct RealRead {
    1: list<double> value,
    2: Status status
}

struct StringRead {
    1: list<string> value,
    2: Status status
}

struct BooleanRead {
    1: list<bool> value,
    2: Status status
}

struct ModelDescription {
    1: string guid,
    2: string author,
    3: string model_name,
    4: string fmi_version,
    5: string description,
    6: string model_identifier,
    7: string generation_tool,
    8: string generation_date_and_time,
    9: optional DefaultExperiment default_experiment,
    10: ModelVariables model_variables,
}
