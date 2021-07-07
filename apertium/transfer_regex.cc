#include <transfer_regex.h>

struct TrieNode {
  UChar32 c;
  bool end;
  std::vector<TrieNode*> next;
};

TrieNode*
add_char(TrieNode* root, UChar32 c)
{
  for (auto node : root->next) {
    if (node->c == c) {
      return node;
    }
  }
  TrieNode* t = new TrieNode;
  t->c = c;
  t->end = false;
  root->next.push_back(t);
  return t;
}

void
add_entry(TrieNode* root, const std::vector<int32_t>& vec)
{
  bool escape = false;
  TrieNode* cur = root;
  for (auto c : vec) {
    if (!escape) {
      if (c == '\\') {
        escape = true;
        continue;
      } else if (c == '.') {
        cur = add_char(cur, '>');
        cur = add_char(cur, '<');
        continue;
      }
    }
    escape = false;
    cur = add_char(cur, c);
  }
  cur->end = true;
}

UString
unbuildTrie(TrieNode* root)
{
  UString single;
  single += '[';
  std::vector<UString> groups;
  int single_count = 0;
  for (auto it : root->next) {
    if (it->end && it->next.empty()) {
      single += it->c;
      single_count++;
    } else {
      groups.push_back(unbuildTrie(it));
    }
  }
  if (single_count > 0) {
    if (single_count == 1) {
      groups.push_back(single.substr(1));
    } else {
      single += ']';
      groups.push_back(single);
    }
  }
  UString ret;
  if (root->c == '+' || root->c == '*' || root->c == '?' || root->c == '.') {
    ret += '\\';
  }
  ret += root->c;
  if (groups.empty()) {
    return ret;
  } else if (groups.size() == 1) {
    if (root->end && groups[0].size() > 1 && groups[0][0] != '(') {
      ret += '('; ret += '?'; ret += ':';
      ret += groups[0];
      ret += ')';
    } else {
      ret += groups[0];
    }
  } else {
    ret += '('; ret += '?'; ret += ':';
    for (size_t i = 0; i < groups.size(); i++) {
      if (i > 0) {
        ret += '|';
      }
      ret += groups[i];
    }
    ret += ')';
  }
  if (root->end) {
    ret += '?';
  }
  return ret;
}

UString
optimize_regex(const std::vector<UString>& options)
{
  TrieNode* root = new TrieNode;
  root->c = '<';
  root->end = false;
  std::vector<int32_t> v;
  for (auto& s : options) {
    v.clear();
    ustring_to_vec32(s, v);
    add_entry(root, v);
  }
  UString ret;
  ret += '(';
  ret.append(unbuildTrie(root));
  ret += '>';
  ret += ')';
  return ret;
}
