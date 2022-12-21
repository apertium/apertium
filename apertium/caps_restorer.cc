#include <apertium/caps_restorer.h>
#include <lttoolbox/compression.h>
#include <lttoolbox/string_utils.h>

CapsRestorer::CapsRestorer()
{
}

CapsRestorer::~CapsRestorer()
{
}

void
CapsRestorer::load(FILE* input)
{
  Compression::multibyte_read(input); // source caps mode (currently unused)

  alpha.read(input);
  any_char = alpha("<ANY_CHAR>"_u);
  any_upper = alpha("<ANY_UPPER>"_u);
  any_lower = alpha("<ANY_LOWER>"_u);
  any_tag = alpha("<ANY_TAG>"_u);
  word_boundary = alpha("<$>"_u);
  null_boundary = alpha("<$$>"_u);

  trans.read(input, alpha);

  size_t lim = Compression::multibyte_read(input);
  for (size_t i = 0; i < lim; i++) {
    UString tag = "<"_u + StringUtils::itoa(i) + ">"_u;
    weights[tag] = Compression::long_multibyte_read(input);
  }

  initial_state.init(trans.getInitial());
  finals.insert(trans.getFinals().begin(), trans.getFinals().end());
}

void
CapsRestorer::process(InputFile& input, UFILE* output)
{
  current_state = initial_state;
  current_state.step_optional(null_boundary);
  while (!input.eof()) {
    if (input.peek() == '\0') {
      output_all(output);
      current_state = initial_state;
      current_state.step_optional(null_boundary);
      u_fputc(input.get(), output);
      u_fflush(output);
      continue;
    }
    read_word(input);
    current_state.step_optional(word_boundary);
    if (current_state.isFinal(finals)) {
      auto path_set = current_state.filterFinalsLRX(finals, alpha, escaped_chars, false, false, 0);
      sorted_vector<UString> seen_rules;
      for (auto& it : path_set) {
        if (seen_rules.count(it.first)) continue;
        seen_rules.insert(it.first);

        auto& path = it.second;
        for (size_t i = 1; i <= path.size(); i++) {
          LU& w = words[words.size()-i];
          const UString& p = path[path.size()-i];
          if (p == "<AA>"_u) {
            w.w_AA += weights[it.first];
          } else if (p == "<Aa>"_u) {
            w.w_Aa += weights[it.first];
          } else if (p == "<aa>"_u) {
            w.w_aa += weights[it.first];
          } else if (p == "<dix>"_u) {
            w.w_dix += weights[it.first];
          }
        }
      }
    }
    if (current_state.size() == 0) {
      output_all(output);
      current_state = initial_state;
    } else {
      current_state.merge(initial_state);
    }
  }
  output_all(output);
}

void
CapsRestorer::output_all(UFILE* output)
{
  for (auto& w : words) {
    write(w.blank, output);
    if (w.surf.empty()) continue;
    if (w.w_dix >= w.w_AA && w.w_dix >= w.w_Aa && w.w_dix >= w.w_aa) {
    } else if (w.w_aa >= w.w_AA && w.w_aa >= w.w_Aa) {
      w.surf = StringUtils::tolower(w.surf);
    } else if (w.w_Aa >= w.w_AA) {
      w.surf = StringUtils::totitle(w.surf);
    } else {
      w.surf = StringUtils::toupper(w.surf);
    }
    if (w.wblank.empty()) {
      write(w.surf, output);
    } else {
      u_fprintf(output, "%S%S[[/]]", w.wblank.c_str(), w.surf.c_str());
    }
  }
  words.clear();
}

void
CapsRestorer::read_seg(InputFile& input, UString& seg)
{
  bool escaped = false;
  while (!input.eof()) {
    UChar32 c = input.get();
    if (escaped) {
      seg += c;
      escaped = false;
    } else if (c == '\\') {
      seg += c;
      escaped = true;
    } else if (c == '<') {
      seg += input.readBlock('<', '>');
    } else if (c == '/' || c == '$') {
      input.unget(c);
      break;
    } else {
      seg += c;
    }
  }
}

void
CapsRestorer::step(int32_t sym)
{
  if (sym < 0) {
    current_state.step(sym, any_tag);
  } else if (sym == 0) {
    current_state.step(any_tag);
  } else if (u_isupper(sym)) {
    current_state.step(sym, any_char, any_upper);
  } else {
    current_state.step(sym, any_char, any_lower);
  }
}

void
CapsRestorer::read_word(InputFile& input)
{
  LU cur;
  cur.blank = input.readBlank(false);
  if (input.peek() == '[') {
    input.get();
    input.get();
    cur.wblank = input.finishWBlank();
  }
  if (input.peek() != '^') {
    words.push_back(std::move(cur));
    return;
  }
  input.get();
  read_seg(input, cur.form);
  if (input.get() == '$') {
    cur.form.swap(cur.surf);
  } else {
    UString temp;
    UChar32 c;
    do {
      read_seg(input, temp);
      c = input.get();
    } while (c == '/');
    cur.surf.swap(temp);
  }

  UString src_caps = "aa/aa"_u;
  if (!cur.wblank.empty()) {
    auto pieces = StringUtils::split(cur.wblank.substr(2, cur.wblank.size()-4), "; "_u);
    for (auto& it : pieces) {
      if (StringUtils::startswith(it, "c:"_u)) {
        src_caps = it.substr(2);
        break;
      }
    }
  }
  for (auto& c : src_caps) {
    if (c == '/') {
      current_state.step(c);
    } else {
      step(c);
    }
  }
  current_state.step('/');
  for (auto& sym : alpha.tokenize(cur.form)) {
    step(sym);
  }
  current_state.step('/');
  for (auto& sym : alpha.tokenize(cur.surf)) {
    step(sym);
  }
  current_state.step(word_boundary);

  words.push_back(std::move(cur));
}
