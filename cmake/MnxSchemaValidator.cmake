if(MNXDOM_USE_LEGACY_SCHEMA_VALIDATOR)
    set(validator_package nlohmann_json_schema_validator)
    set(MNXDOM_VALIDATOR_TARGET nlohmann_json_schema_validator::validator)
    set(MNXDOM_VALIDATOR_LIBRARY nlohmann_json_schema_validator)
    target_compile_definitions(mnxdom PRIVATE MNXDOM_USE_LEGACY_SCHEMA_VALIDATOR)
else()
    set(validator_package jsonschema)
    set(MNXDOM_VALIDATOR_TARGET jsonschema::jsonschema)
    set(MNXDOM_VALIDATOR_LIBRARY jsonschema)
endif()

if(USE_SYSTEM_JSON_SCHEMA_VALIDATOR)
    find_package(${validator_package} CONFIG REQUIRED)
    message(STATUS "mnxdom schema validator: installed ${validator_package} from ${${validator_package}_DIR}")
elseif(MNXDOM_USE_LEGACY_SCHEMA_VALIDATOR)
    set(JSON_VALIDATOR_BUILD_TESTS OFF)
    set(JSON_VALIDATOR_BUILD_EXAMPLES OFF)
    set(JSON_VALIDATOR_INSTALL ON)
    set(nlohmann_json_VERSION ${NLOHMANN_JSON_VERSION})
    FetchContent_Declare(json_schema_validator
        GIT_REPOSITORY https://github.com/pboettch/json-schema-validator
        GIT_TAG c780404a84dd9ba978ba26bc58d17cb43fa7bc80) # 2.4.0
    FetchContent_MakeAvailable(json_schema_validator)
    message(STATUS "mnxdom schema validator: fetched json-schema-validator")
else()
    if(CMAKE_VERSION VERSION_LESS 3.21)
        message(FATAL_ERROR "Fetching json-schema-cpp requires CMake 3.21 or newer")
    endif()
    cmake_policy(SET CMP0097 NEW) # GIT_SUBMODULES "" skips the upstream test suite
    set(JSONSCHEMA_BUILD_TESTS OFF)
    set(JSONSCHEMA_BUILD_CLI OFF)
    set(JSONSCHEMA_BUILD_BOWTIE OFF)
    set(JSONSCHEMA_INSTALL ON)
    FetchContent_Declare(json_schema_cpp
        GIT_REPOSITORY https://github.com/rpatters1/json-schema-cpp.git
        GIT_TAG aab374748c580e94e60778a6866b3c4231063ff1
        GIT_SUBMODULES "")
    FetchContent_MakeAvailable(json_schema_cpp)
    message(STATUS "mnxdom schema validator: fetched json-schema-cpp")
endif()

if(MNXDOM_USE_LEGACY_SCHEMA_VALIDATOR AND NOT TARGET ${MNXDOM_VALIDATOR_TARGET} AND TARGET nlohmann_json_schema_validator)
    set(MNXDOM_VALIDATOR_TARGET nlohmann_json_schema_validator)
endif()
if(NOT TARGET ${MNXDOM_VALIDATOR_TARGET})
    message(FATAL_ERROR "${validator_package} did not provide the expected validator target ${MNXDOM_VALIDATOR_TARGET}")
endif()
