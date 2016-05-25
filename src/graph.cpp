#include "graph.hpp"

#include "external_types.hpp"

#include <assert.h>

static const DseGraphResult* find_member(const DseGraphResult* result,
                                         const char* name, size_t expected_index) {
  if (expected_index < result->MemberCount()) {
    const rapidjson::Value::Member& member = result->MemberBegin()[expected_index];
    if (member.name == name) {
      return DseGraphResult::to(&member.value);
    }
  }
  rapidjson::Value::ConstMemberIterator i = result->FindMember(name);
  return i != result->MemberEnd() ? DseGraphResult::to(&i->value) : NULL;
}

extern "C" {

CassFuture* cass_session_execute_dse_graph(CassSession* session,
                                           const DseGraphStatement* statement) {
  return cass_session_execute(session, statement->wrapped());
}

DseGraphResultSet* cass_future_get_dse_graph_resultset(CassFuture* future) {
  const CassResult* result = cass_future_get_result(future);
  if (result == NULL) return NULL;
  return DseGraphResultSet::to(new dse::GraphResultSet(result));
}

DseGraphOptions* dse_graph_options_new() {
  return DseGraphOptions::to(new dse::GraphOptions());
}

void dse_graph_options_free(DseGraphOptions* options) {
  delete options->from();
}

CassError dse_graph_options_set_graph_language(DseGraphOptions* options,
                                               const char* language) {
  options->set_graph_language(language);
  return CASS_OK;
}

CassError dse_graph_options_set_graph_language_n(DseGraphOptions* options,
                                                 const char* language, size_t language_length) {
  options->set_graph_language(std::string(language, language_length));
  return CASS_OK;
}

CassError dse_graph_options_set_graph_source(DseGraphOptions* options,
                                             const char* source) {
  options->set_graph_source(source);
  return CASS_OK;
}

CassError dse_graph_options_set_graph_source_n(DseGraphOptions* options,
                                               const char* source, size_t source_length) {
  options->set_graph_source(std::string(source, source_length));
  return CASS_OK;
}

CassError dse_graph_options_set_graph_name(DseGraphOptions* options,
                                           const char* name) {
  options->set_graph_name(name);
  return CASS_OK;
}

CassError dse_graph_options_set_graph_name_n(DseGraphOptions* options,
                                             const char* name, size_t name_length) {
  options->set_graph_name(std::string(name, name_length));
  return CASS_OK;
}

DseGraphStatement* dse_graph_statement_new(const char* query,
                                           const DseGraphOptions* options) {
  return dse_graph_statement_new_n(query, strlen(query),
                                   options);
}

DseGraphStatement* dse_graph_statement_new_n(const char* query,
                                             size_t query_length,
                                             const DseGraphOptions* options) {
  return DseGraphStatement::to(new dse::GraphStatement(query, query_length,
                                                       options));
}

void dse_graph_statement_free(DseGraphStatement* statement) {
  delete statement->from();
}

CassError dse_graph_statement_bind_values(DseGraphStatement* statement,
                                          const DseGraphObject* values) {
  if (values != NULL && !values->is_complete()) {
    return CASS_ERROR_LIB_BAD_PARAMS;
  }
  return statement->bind_values(values);
}

DseGraphObject* dse_graph_object_new() {
  return DseGraphObject::to(new dse::GraphObject());
}

void dse_graph_object_free(DseGraphObject* object) {
  delete object->from();
}

void dse_graph_object_reset(DseGraphObject* object) {
  object->reset();
}

void dse_graph_object_finish(DseGraphObject* object) {
  object->finish();
}

CassError dse_graph_object_add_null(DseGraphObject* object,
                                    const char* name) {
  return dse_graph_object_add_null_n(object,
                                     name, strlen(name));
}

CassError dse_graph_object_add_null_n(DseGraphObject* object,
                                      const char* name,
                                      size_t name_length) {
  if (object->is_complete()) {
    return CASS_ERROR_LIB_BAD_PARAMS;
  }
  object->add_key(name, name_length);
  object->add_null();
  return CASS_OK;
}

CassError dse_graph_object_add_bool(DseGraphObject* object,
                                     const char* name,
                                     cass_bool_t value) {
  return dse_graph_object_add_bool_n(object,
                                      name, strlen(name),
                                      value);
}

CassError dse_graph_object_add_bool_n(DseGraphObject* object,
                                      const char* name,
                                      size_t name_length,
                                      cass_bool_t value) {
  if (object->is_complete()) {
    return CASS_ERROR_LIB_BAD_PARAMS;
  }
  object->add_key(name, name_length);
  object->add_bool(value);
  return CASS_OK;
}

CassError dse_graph_object_add_int32(DseGraphObject* object,
                                     const char* name,
                                     cass_int32_t value) {
  return dse_graph_object_add_int32_n(object,
                                      name, strlen(name),
                                      value);
}

CassError dse_graph_object_add_int32_n(DseGraphObject* object,
                                       const char* name,
                                       size_t name_length,
                                       cass_int32_t value) {
  if (object->is_complete()) {
    return CASS_ERROR_LIB_BAD_PARAMS;
  }
  object->add_key(name, name_length);
  object->add_int32(value);
  return CASS_OK;
}

CassError dse_graph_object_add_int64(DseGraphObject* object,
                                     const char* name,
                                     cass_int64_t value) {
  return dse_graph_object_add_int64_n(object,
                                      name, strlen(name),
                                      value);
}

CassError dse_graph_object_add_int64_n(DseGraphObject* object,
                                       const char* name,
                                       size_t name_length,
                                       cass_int64_t value) {
  if (object->is_complete()) {
    return CASS_ERROR_LIB_BAD_PARAMS;
  }
  object->add_key(name, name_length);
  object->add_int64(value);
  return CASS_OK;
}

CassError dse_graph_object_add_double(DseGraphObject* object,
                                      const char* name,
                                      cass_double_t value) {
  return dse_graph_object_add_double_n(object,
                                       name, strlen(name),
                                       value);
}

CassError dse_graph_object_add_double_n(DseGraphObject* object,
                                        const char* name,
                                        size_t name_length,
                                        cass_double_t value) {
  if (object->is_complete()) {
    return CASS_ERROR_LIB_BAD_PARAMS;
  }
  object->add_key(name, name_length);
  object->add_double(value);
  return CASS_OK;
}

CassError dse_graph_object_add_string(DseGraphObject* object,
                                      const char* name,
                                      const char* value) {
  return dse_graph_object_add_string_n(object,
                                       name, strlen(name),
                                       value, strlen(value));
}

CassError dse_graph_object_add_string_n(DseGraphObject* object,
                                        const char* name, size_t name_length,
                                        const char* value, size_t value_length) {
  if (object->is_complete()) {
    return CASS_ERROR_LIB_BAD_PARAMS;
  }
  object->add_key(name, name_length);
  object->add_string(value, value_length);
  return CASS_OK;
}

CassError dse_graph_object_add_object(DseGraphObject* object,
                                      const char* name,
                                      const DseGraphObject* value) {
  return dse_graph_object_add_object_n(object,
                                       name, strlen(name),
                                       value);
}

CassError dse_graph_object_add_object_n(DseGraphObject* object,
                                        const char* name,
                                        size_t name_length,
                                        const DseGraphObject* value) {
  if (object->is_complete() || !value->is_complete()) {
    return CASS_ERROR_LIB_BAD_PARAMS;
  }
  object->add_key(name, name_length);
  object->add_writer(value, rapidjson::kObjectType);
  return CASS_OK;
}

CassError dse_graph_object_add_array(DseGraphObject* object,
                                     const char* name,
                                     const DseGraphArray* value) {
  return dse_graph_object_add_array_n(object,
                                      name, strlen(name),
                                      value);
}

CassError dse_graph_object_add_array_n(DseGraphObject* object,
                                       const char* name,
                                       size_t name_length,
                                       const DseGraphArray* value) {
  if (object->is_complete() || !value->is_complete()) {
    return CASS_ERROR_LIB_BAD_PARAMS;
  }
  object->add_key(name, name_length);
  object->add_writer(value, rapidjson::kArrayType);
  return CASS_OK;
}

DseGraphArray* dse_graph_array_new() {
  return DseGraphArray::to(new DseGraphArray());
}

void dse_graph_array_free(DseGraphArray* array) {
  delete array->from();
}

void dse_graph_array_reset(DseGraphArray* array) {
  array->reset();
}

void dse_graph_array_finish(DseGraphArray* array) {
  array->finish();
}

CassError dse_graph_array_add_null(DseGraphArray* array) {
  if (array->is_complete()) {
    return CASS_ERROR_LIB_BAD_PARAMS;
  }
  array->add_null();
  return CASS_OK;
}

CassError dse_graph_array_add_bool(DseGraphArray* array,
                                   cass_bool_t value) {
  if (array->is_complete()) {
    return CASS_ERROR_LIB_BAD_PARAMS;
  }
  array->add_bool(value);
  return CASS_OK;
}

CassError dse_graph_array_add_int32(DseGraphArray* array,
                                    cass_int32_t value) {
  if (array->is_complete()) {
    return CASS_ERROR_LIB_BAD_PARAMS;
  }
  array->add_int32(value);
  return CASS_OK;
}

CassError dse_graph_array_add_int64(DseGraphArray* array,
                                    cass_int64_t value) {
  if (array->is_complete()) {
    return CASS_ERROR_LIB_BAD_PARAMS;
  }
  array->add_int64(value);
  return CASS_OK;
}

CassError dse_graph_array_add_double(DseGraphArray* array,
                                     cass_double_t value) {
  if (array->is_complete()) {
    return CASS_ERROR_LIB_BAD_PARAMS;
  }
  array->add_double(value);
  return CASS_OK;
}

CassError dse_graph_array_add_string(DseGraphArray* array,
                                     const char* value) {
  return dse_graph_array_add_string_n(array, value, strlen(value));
}

CassError dse_graph_array_add_string_n(DseGraphArray* array,
                                       const char* value,
                                       size_t value_length) {
  if (array->is_complete()) {
    return CASS_ERROR_LIB_BAD_PARAMS;
  }
  array->add_string(value, value_length);
  return CASS_OK;
}

CassError dse_graph_array_add_object(DseGraphArray* array,
                                     const DseGraphObject* value) {
  if (array->is_complete() || !value->is_complete()) {
    return CASS_ERROR_LIB_BAD_PARAMS;
  }
  array->add_writer(value, rapidjson::kObjectType);
  return CASS_OK;
}

CassError dse_graph_array_add_array(DseGraphArray* array,
                                    const DseGraphArray* value) {
  if (array->is_complete() || !value->is_complete()) {
    return CASS_ERROR_LIB_BAD_PARAMS;
  }
  array->add_writer(value, rapidjson::kArrayType);
  return CASS_OK;
}

void dse_graph_resultset_free(DseGraphResultSet* resultset) {
  delete resultset->from();
}

size_t dse_graph_resultset_count(DseGraphResultSet* resultset) {
  return resultset->count();
}

const DseGraphResult* dse_graph_resultset_next(DseGraphResultSet* resultset) {
  return DseGraphResult::to(resultset->next());
}

DseGraphResultType dse_graph_result_type(const DseGraphResult* result) {
  switch (result->GetType()) {
    case rapidjson::kNullType: return DSE_GRAPH_RESULT_TYPE_NULL;
    case rapidjson::kFalseType: // Intentional fallthrough
    case rapidjson::kTrueType: return DSE_GRAPH_RESULT_TYPE_BOOL;
    case rapidjson::kNumberType: return DSE_GRAPH_RESULT_TYPE_NUMBER;
    case rapidjson::kStringType: return DSE_GRAPH_RESULT_TYPE_STRING;
    case rapidjson::kObjectType: return DSE_GRAPH_RESULT_TYPE_OBJECT;
    case rapidjson::kArrayType: return DSE_GRAPH_RESULT_TYPE_ARRAY;
  }

  // Path should never be executed
  assert(false);
  return DSE_GRAPH_RESULT_TYPE_NULL;
}

cass_bool_t dse_graph_result_is_bool(const DseGraphResult* result) {
  return result->IsBool() ? cass_true : cass_false;
}

cass_bool_t dse_graph_result_is_int32(const DseGraphResult* result) {
  return result->IsInt() ? cass_true : cass_false;
}

cass_bool_t dse_graph_result_is_int64(const DseGraphResult* result) {
  return result->IsInt64() ? cass_true : cass_false;
}

cass_bool_t dse_graph_result_is_double(const DseGraphResult* result) {
  return result->IsDouble() ? cass_true : cass_false;
}

cass_bool_t dse_graph_result_is_string(const DseGraphResult* result) {
  return result->IsString() ? cass_true : cass_false;
}

cass_bool_t dse_graph_result_is_object(const DseGraphResult* result) {
  return result->IsObject() ? cass_true : cass_false;
}

cass_bool_t dse_graph_result_is_array(const DseGraphResult* result) {
  return result->IsArray() ? cass_true : cass_false;
}

cass_bool_t dse_graph_result_get_bool(const DseGraphResult* result) {
  return result->GetBool() ? cass_true : cass_false;
}

cass_int32_t dse_graph_result_get_int32(const DseGraphResult* result) {
  return result->GetInt();
}

cass_int64_t dse_graph_result_get_int64(const DseGraphResult* result) {
  return result->GetInt64();
}

cass_double_t dse_graph_result_get_double(const DseGraphResult* result) {
  return result->GetDouble();
}

const char* dse_graph_result_get_string(const DseGraphResult* result,
                                        size_t* length) {
  if (length != NULL) {
    *length = result->GetStringLength();
  }
  return result->GetString();
}

#define CHECK_FIND_MEMBER(dest, name, expected_index) do { \
  const DseGraphResult* src = find_member(result, name, expected_index); \
  if (src != NULL) { \
  (dest) = src; \
} else { \
  return CASS_ERROR_LIB_BAD_PARAMS; \
} \
} while (0)

CassError dse_graph_result_as_edge(const DseGraphResult* result,
                                   DseGraphEdgeResult* edge) {
  if (!result->IsObject()) {
    return CASS_ERROR_LIB_BAD_PARAMS;
  }

  CHECK_FIND_MEMBER(edge->id,               "id",         0);
  CHECK_FIND_MEMBER(edge->label,            "label",      1);
  CHECK_FIND_MEMBER(edge->type,             "type",       2);
  CHECK_FIND_MEMBER(edge->in_vertex_label,  "inVLabel",   3);
  CHECK_FIND_MEMBER(edge->out_vertex_label, "outVLabel",  4);
  CHECK_FIND_MEMBER(edge->in_vertex,        "inV",        5);
  CHECK_FIND_MEMBER(edge->out_vertex,       "outV",       6);
  CHECK_FIND_MEMBER(edge->properties,       "properties", 7);

  return CASS_OK;
}

CassError dse_graph_result_as_vertex(const DseGraphResult* result,
                                      DseGraphVertexResult* vertex) {
  if (!result->IsObject()) {
    return CASS_ERROR_LIB_BAD_PARAMS;
  }

  CHECK_FIND_MEMBER(vertex->id,         "id",         0);
  CHECK_FIND_MEMBER(vertex->label,      "label",      1);
  CHECK_FIND_MEMBER(vertex->type,       "type",       2);
  CHECK_FIND_MEMBER(vertex->properties, "properties", 3);

  return CASS_OK;
}

CassError dse_graph_result_as_path(const DseGraphResult* result,
                                    DseGraphPathResult* path) {
  if (!result->IsObject()) {
    return CASS_ERROR_LIB_BAD_PARAMS;
  }

  CHECK_FIND_MEMBER(path->labels,  "labels",  0);
  CHECK_FIND_MEMBER(path->objects, "objects", 1);

  return CASS_OK;
}

#undef CHECK_FIND_MEMBER

size_t dse_graph_result_member_count(const DseGraphResult* result) {
  return result->MemberCount();
}

const char* dse_graph_result_member_key(const DseGraphResult* result,
                                        size_t index,
                                        size_t* length) {
 const rapidjson::Value& key = result->MemberBegin()[index].name;
 if (length != NULL) {
   *length = key.GetStringLength();
 }
 return key.GetString();
}

const DseGraphResult* dse_graph_result_member_value(const DseGraphResult* result,
                                                    size_t index) {
  return DseGraphResult::to(&result->MemberBegin()[index].value);
}

size_t dse_graph_result_element_count(const DseGraphResult* result) {
  return result->Size();
}

const DseGraphResult* dse_graph_result_element(const DseGraphResult* result,
                                               size_t index) {
  return DseGraphResult::to(&result->Begin()[index]);
}

} // extern "C"

namespace dse {

const GraphResult* GraphResultSet::next() {
  if (cass_iterator_next(rows_)) {
    const CassRow* row = cass_iterator_get_row(rows_);
    if (row == NULL) return NULL;

    const CassValue* value = cass_row_get_column_by_name(row, "gremlin");
    if (value == NULL) return NULL;

    const char* json;
    size_t length;
    cass_value_get_string(value, &json, &length);

    // A copy is needed to make this null-terminated, but it
    // allows for insitu parsing.
    json_.assign(json, length);
    if (document.ParseInsitu(&json_[0]).HasParseError()) {
      return NULL;
    }

    rapidjson::Value::ConstMemberIterator i = document.FindMember("result");
    return i != document.MemberEnd() ? &i->value : NULL;
  }
  return NULL;
}

} // namespace dse