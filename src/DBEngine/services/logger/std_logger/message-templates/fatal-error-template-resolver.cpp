#include "./public-api.hpp"

std::string
MessageTemplateResolver::resolveFatalErrorMessageTemplate(const StatusCode::FatalErrorCode& error_code,
                                                          DB_Types::status_context_t error_context = std::nullopt) {

  std::vector<std::string> resolved_context;
  if (error_context != std::nullopt) { resolved_context = error_context.value(); }
  switch (error_code) {
    case StatusCode::FatalErrorCode::FILEOPS_GenericFileIOFailure: {
      return MessageTemplateResolver::injectContext("An unknown IO error occured. Error message: {}", resolved_context);
    }
    case StatusCode::FatalErrorCode::FILEOPS_ColOffsetRegionHasMoreColsThanAllowed: {
      return MessageTemplateResolver::injectContext("Table {} Has more than 256 columns on the disk, which is illegal.",
                                                    resolved_context);
    }
    case StatusCode::FatalErrorCode::NOCONTX_NULLPTR_InMemoryPrimaryKeyHashMapInitializationFailure: {
      return "One of the pointers responsible for primary key indexing was nullptr.";
    }
    case StatusCode::FatalErrorCode::FILEOPS_UnknownTableFileFormat: {
      return MessageTemplateResolver::injectContext("Unknown table file format for table {}.", resolved_context);
    }
  }
}
