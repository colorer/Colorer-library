#ifndef _COLORER_BASE_ENTITY_RESOLVER_H_
#define _COLORER_BASE_ENTITY_RESOLVER_H_

#include <xercesc/util/XMLEntityResolver.hpp>

class BaseEntityResolver : public xercesc::XMLEntityResolver
{
 public:
  BaseEntityResolver() = default;
  ~BaseEntityResolver() override = default;
  xercesc::InputSource* resolveEntity(xercesc::XMLResourceIdentifier* resourceIdentifier) override;
};

#endif
