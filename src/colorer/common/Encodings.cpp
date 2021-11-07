#include "colorer/common/Encodings.h"
#include <unicode/ucnv.h>
#include "colorer/Exception.h"

uUnicodeString Encodings::toUnicodeString(char* data, int32_t len)
{
  const char* encoding;

  // try detect encoding
  UErrorCode status {U_ZERO_ERROR};
  int32_t signatureLength;
  encoding = ucnv_detectUnicodeSignature(data, len, &signatureLength, &status);
  if (U_FAILURE(status)) {
    spdlog::error("Encodings: Error \"{0}\" from ucnv_detectUnicodeSignature()\n",
                  u_errorName(status));
    throw Exception("Error from ucnv_detectUnicodeSignature");
  }
  if (encoding == nullptr) {
    encoding = ENC_UTF8;
  }

  return std::make_unique<UnicodeString>(data + signatureLength, len - signatureLength, encoding);
}