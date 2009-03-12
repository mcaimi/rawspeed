#include "StdAfx.h"
#include "Camera.h"

Camera::Camera(xmlDocPtr doc, xmlNodePtr cur) {
  xmlChar *key;
  key = xmlGetProp(cur,(const xmlChar *)"make");
  if (!key)
    ThrowCME("Camera XML Parser: \"make\" attribute not found.");
  make = string((const char*)key);
  xmlFree(key);

  key = xmlGetProp(cur,(const xmlChar *)"model");
  if (!key)
    ThrowCME("Camera XML Parser: \"model\" attribute not found.");
  model = string((const char*)key);
  xmlFree(key);

  cur = cur->xmlChildrenNode;
  while (cur != NULL) {
    parseCameraChild(doc, cur);
    cur = cur->next;
  }
}

Camera::~Camera(void) {
}

void Camera::parseCameraChild( xmlDocPtr doc, xmlNodePtr cur )
{
  if (!xmlStrcmp(cur->name, (const xmlChar *) "CFA")) {
    if( 2 != getAttributeAsInt(cur,cur->name,"width"))
      ThrowCME("Unsupported CFA size in camera %s %s", make.c_str(), model.c_str());
    if( 2 != getAttributeAsInt(cur,cur->name,"height"))
      ThrowCME("Unsupported CFA size in camera %s %s", make.c_str(), model.c_str());

    cur = cur->xmlChildrenNode;
    while (cur != NULL) {
      parseCFA(doc, cur);
      cur = cur->next;
    }
    
    return;
  }

  if (!xmlStrcmp(cur->name, (const xmlChar *) "Crop")) {
    cropPos.x = getAttributeAsInt(cur,cur->name,"x");
    cropPos.y = getAttributeAsInt(cur,cur->name,"y");

    cropSize.x = getAttributeAsInt(cur,cur->name,"width");
    cropSize.y = getAttributeAsInt(cur,cur->name,"height");
    return;
  }

  if (!xmlStrcmp(cur->name, (const xmlChar *) "Sensor")) {
    black = getAttributeAsInt(cur,cur->name,"black");
    white = getAttributeAsInt(cur,cur->name,"white");
    return;
  }

  if (!xmlStrcmp(cur->name, (const xmlChar *) "BlackAreas")) {
    cur = cur->xmlChildrenNode;
    while (cur != NULL) {
      parseBlackAreas(doc, cur);
      cur = cur->next;
    }
    return;
  }
}

void Camera::parseCFA( xmlDocPtr doc, xmlNodePtr cur )
{
  xmlChar *key;
  if (!xmlStrcmp(cur->name, (const xmlChar *) "Color")) {
    int x = getAttributeAsInt(cur,cur->name,"x");
    if (x<0 || x>1) {
      ThrowCME("Invalid x coordinate in CFA array of in camera %s %s", make.c_str(), model.c_str()); 
    }

    int y = getAttributeAsInt(cur,cur->name,"y");
    if (y<0 || y>1) {
      ThrowCME("Invalid y coordinate in CFA array of in camera %s %s", make.c_str(), model.c_str()); 
    }

    key = xmlNodeListGetString(doc, cur->children, 1);
    if (!xmlStrcmp(key, (const xmlChar *) "GREEN"))
      cfa.setColorAt(iPoint2D(x,y),CFA_GREEN);
    else if (!xmlStrcmp(key, (const xmlChar *) "RED"))
      cfa.setColorAt(iPoint2D(x,y),CFA_RED);
    else if (!xmlStrcmp(key, (const xmlChar *) "BLUE"))
      cfa.setColorAt(iPoint2D(x,y),CFA_BLUE);

    xmlFree(key);

  }
}

void Camera::parseBlackAreas( xmlDocPtr doc, xmlNodePtr cur )
{
  if (!xmlStrcmp(cur->name, (const xmlChar *) "Vertical")) {

    int x = getAttributeAsInt(cur,cur->name,"x");
    if (x<0) {
      ThrowCME("Invalid x coordinate in vertical BlackArea of in camera %s %s", make.c_str(), model.c_str()); 
    }

    int w = getAttributeAsInt(cur,cur->name,"width");
    if (w<0) {
      ThrowCME("Invalid width in vertical BlackArea of in camera %s %s", make.c_str(), model.c_str()); 
    }

    blackAreas.push_back(BlackArea(x,w,true));

  } else if (!xmlStrcmp(cur->name, (const xmlChar *) "Horizontal")) {

      int y = getAttributeAsInt(cur,cur->name,"y");
      if (y<0) {
        ThrowCME("Invalid y coordinate in horizontal BlackArea of in camera %s %s", make.c_str(), model.c_str()); 
      }

      int h = getAttributeAsInt(cur,cur->name,"height");
      if (h<0) {
        ThrowCME("Invalid width in horizontal BlackArea of in camera %s %s", make.c_str(), model.c_str()); 
      }

      blackAreas.push_back(BlackArea(y,h,false));

  }
}

int Camera::StringToInt( const xmlChar *in, const xmlChar *tag, const char* attribute )
{
  int i;

  if (EOF == sscanf_s((const char*)in, "%d", &i))
    ThrowCME("Error parsing attribute %s in tag %s, in camera %s %s.", attribute, tag, make.c_str(), model.c_str());
  
  return i;
}


int Camera::getAttributeAsInt( xmlNodePtr cur , const xmlChar *tag, const char* attribute)
{
  xmlChar *key = xmlGetProp(cur,(const xmlChar *)attribute);

  if (!key)
    ThrowCME("Could not find attribute %s in tag %s, in camera %s %s.", attribute, tag, make.c_str(), model.c_str());

  int i = StringToInt(key,tag,attribute);

  return i;
}
