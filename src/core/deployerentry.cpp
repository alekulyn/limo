#include "deployerentry.hpp"
#include <json/config.h>


Json::Value DeployerEntry::toJson()
{
  if (!isSeparator) {
    return static_cast<DeployerModInfo *>(this)->toJson();
  }
  Json::Value json_object;
  json_object["name"] = name;
  // json_object["expanded"] = name;
  return json_object;
}

Json::Value DeployerModInfo::toJson()
{
  Json::Value json_object;
  json_object["id"] = id;
  json_object["status"] = enabled;
  return json_object;
}
