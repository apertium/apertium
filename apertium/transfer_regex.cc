#include <transfer_regex.h>

struct TrieNode {
  UChar32 c;
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
  add_char(cur, '\0');
}

UString
unbuildTrie(TrieNode* root)
{
  UString single;
  single += '[';
  std::vector<UString> groups;
  bool end = false;
  int single_count = 0;
  for (auto it : root->next) {
    if (it->next.empty()) {
      end = true;
    } else if (it->next.size() == 1 && it->next[0]->c == '\0') {
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
  ret += root->c;
  if (groups.empty()) {
    return ret;
  } else if (groups.size() == 1) {
    ret += groups[0];
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
  if (end) {
    ret += '?';
  }
  return ret;
}

UString
optimize_regex(const std::vector<UString>& options)
{
  TrieNode* root = new TrieNode;
  root->c = '<';
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
