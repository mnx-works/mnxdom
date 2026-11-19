#include <mnxdom/mnxdom.h>

int main()
{
    mnx::Document document;
    mnx::validation::SchemaValidationOptions options;
    options.jsonSchema = R"({"type":"array"})";
    return mnx::validation::schemaValidate(document, options).errors.empty() ? 1 : 0;
}
