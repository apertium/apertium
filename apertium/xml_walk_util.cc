#include <apertium/xml_walk_util.h>

children::children(xmlNode* node_)
  : node(node_), cur(node->children)
{
  while (cur && cur->type != XML_ELEMENT_NODE) {
    cur = cur->next;
  }
}

children::children(const children& it)
  : node(it.node), cur(it.cur)
{}

children::~children()
{} // we don't own the pointers, so we don't delete them

children&
children::operator++()
{
  if (node && cur) {
    cur = cur->next;
    while (cur && cur->type != XML_ELEMENT_NODE) {
      cur = cur->next;
    }
  }
  return *this;
}

children
children::begin()
{
  return children(node);
}

children
children::end()
{
  children ret(node);
  ret.cur = nullptr;
  return ret;
}

bool
children::operator!=(const children& other) const
{
  return node != other.node || cur != other.cur;
}

bool
children::operator==(const children& other) const
{
  return node == other.node && cur == other.cur;
}

UString
getattr(xmlNode* node, const char* attr)
{
  for (xmlAttr* i = node->properties; i != NULL; i = i->next) {
    if (!xmlStrcmp(i->name, (const xmlChar*) attr)) {
      return to_ustring((const char*) i->children->content);
    }
  }
  return ""_u;
}
