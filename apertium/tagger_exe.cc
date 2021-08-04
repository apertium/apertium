#include <apertium/tagger_exe.h>

#include <lttoolbox/match_state2.h>
#include <lttoolbox/string_utils.h>

using namespace std;
using namespace Apertium;

void
TaggerExe::build_match_finals()
{
  for (uint64_t i = 0; i < tde.finals_count; i++) {
    match_finals[tde.finals[i].i1] = tde.finals[i].i2;
  }
}

StreamedType
TaggerExe::read_streamed_type(InputFile& input)
{
  StreamedType ret;
  ret.TheString = input.readBlank(true);
  if (!input.eof() && input.peek() == '^') {
    input.get();
    ret.TheLexicalUnit = LexicalUnit();
    UChar32 c = input.get();
    while (c != '/' && c != '$') {
      ret.TheLexicalUnit->TheSurfaceForm += c;
      if (c == '\\') {
        ret.TheLexicalUnit->TheSurfaceForm += input.get();
      }
      c = input.get();
    }
    // maybe error here if surface form is empty or we hit $
    c = input.get();
    if (c == '*') {
      input.readBlock(c, '$');
    } else {
      input.unget(c);
      do {
        ret.TheLexicalUnit->TheAnalyses.push_back(Analysis());
        ret.TheLexicalUnit->TheAnalyses.back().read(input);
        c = input.get();
      } while (c == '/');
      // error if c != $
    }
  }
  return ret;
}

TaggerWord*
TaggerExe::read_tagger_word(InputFile& input)
{
  if (!tagger_word_buffer.empty()) {
    TaggerWord* ret = tagger_word_buffer[0];
    tagger_word_buffer.erase(tagger_word_buffer.begin());
    if (ret->isAmbiguous()) {
      for (uint64_t i = 0; i < tde.discard_count; i++) {
        // TODO: have TaggerWord accept UString_view
        UString temp = UString{tde.str_write.get(tde.discard[i])};
        ret->discardOnAmbiguity(temp);
      }
    }
    return ret;
  }

  if (input.eof()) {
    return nullptr;
  }

  size_t index = 0;
  tagger_word_buffer.push_back(new TaggerWord());
  tagger_word_buffer[index]->add_ignored_string(input.readBlank(true));
  UChar32 c = input.get();
  if (input.eof() || (null_flush && c == '\0')) {
    end_of_file = true;
    //tagger_word_buffer[index]->add_tag(ca_tag_keof, ""_u, tde.prefer_rules);
  } else { // c == ^
    UString buf = input.readBlock('^', '$');
    if (buf.back() != '$' &&
        (input.eof() || (null_flush && input.peek() == '\0'))) {
      tagger_word_buffer[index]->add_ignored_string(buf);
      //tagger_word_buffer[index]->add_tag(ca_tag_keof, ""_u, tde.prefer_rules);
      return read_tagger_word(input);
    }
    buf = buf.substr(1, buf.size()-2);
    vector<UString_view> pieces = StringUtils::split_escape(buf, '/');
    UString surf = UString{pieces[0]};
    tagger_word_buffer[index]->set_superficial_form(surf);
    if (pieces.size() > 1) {
      for (auto& it : pieces) {
        index = 0;
        vector<UString_view> segments = StringUtils::split_escape(it, '+');
        MatchState2 state(&tde.trans);
        size_t start = 0;
        int tag = -1;
        size_t last_pos = 0;
        for (size_t i = 0; i < segments.size(); i++) {
          if (i != 0) {
            state.step('+');
          }
          state.step(segments[i], tde.alpha);
          int val = state.classifyFinals(match_finals);
          if (val != -1) {
            tag = val;
            last_pos = i+1;
          }
          if (last_pos == start &&
              (state.empty() || i == segments.size() - 1)) {
            UString tmp = UString{segments[i]};
            for (size_t j = i+1; j < segments.size(); j++) {
              tmp += '+';
              tmp += segments[j];
            }
            /* if (debug) {
              cerr<<"Warning: There is not coarse tag for the fine tag '" << tmp <<"'\n";
              cerr<<"         This is because of an incomplete tagset definition or a dictionary error\n";
              }*/
            //tagger_word_buffer[index]->add_tag(ca_tag_kundef, tmp, tde.prefer);
            break;
          } else if (state.empty() || (i + 1 == segments.size() && val == -1)) {
            UString tmp;
            for (size_t j = start; j < last_pos; j++) {
              if (!tmp.empty()) {
                tmp += '+';
              }
              tmp += segments[j];
            }
            // tagger_word_buffer[index]->add_tag(tag, tmp, tde.prefer);
            if (last_pos < segments.size()) {
              start = last_pos;
              tagger_word_buffer[index]->set_plus_cut(true);
              index++;
              if (index >= tagger_word_buffer.size()) {
                tagger_word_buffer.push_back(new TaggerWord(true));
              }
              state.clear();
            }
            i = start - 1;
          }
          if (tag == -1) {
            // tag = ca_tag_kundef;
            /* if (debug) {
              cerr<<"Warning: There is not coarse tag for the fine tag '" << tmp <<"'\n";
              cerr<<"         This is because of an incomplete tagset definition or a dictionary error\n";
              }*/
          }
          UString tmp;
          for (size_t j = start; j < segments.size(); j++) {
            if (!tmp.empty()) {
              tmp += '+';
            }
            tmp += segments[j];
          }
          // tagger_word_buffer[index]->add_tag(tag, tmp, tde.prefer);
        }
      }
    }
  }
  return read_tagger_word(input);
}

void
TaggerExe::tag_hmm(InputFile& input, UFILE* output)
{
  build_match_finals();

  vector<vector<double>> alpha(2, vector<double>(tde.N));
  vector<vector<vector<uint64_t>>> best(2, vector<vector<uint64_t>>(tde.N));

  set<uint64_t> tags, pretags;

  //tags.insert(eos);
  //alpha[0][eos] = 1;

  vector<TaggerWord> words;
  TaggerWord* cur_word = read_tagger_word(input);

  while (cur_word) {
    words.push_back(*cur_word);

    pretags.swap(tags);
    //tags = cur_word->get_tags();
    if (tags.empty()) {
      // tags = tde.getOpenClass();
    }

    uint64_t cls = tde.get_ambiguity_class(tags);

    //clear_array_double(&alpha[nwpend%2][0], N);
    //clear_array_vector(&best[nwpend%2][0], N);

    if (cls < tde.output_count) {
      // if it's a new ambiguity class, weights will all be 0
      for (auto& i : tags) {
        for (auto& j : pretags) {
          int loc = words.size() % 2;
          double x = alpha[1-loc][j] * tde.getA(j, i) * tde.getB(i, cls);
          if (alpha[loc][i] <= x) {
            if (words.size() > 1) {
              best[loc][i] = best[1-loc][j];
            }
            best[loc][i].push_back(i);
            alpha[loc][i] = x;
          }
        }
      }
    }

    if (tags.size() == 1) {
      uint64_t tag = *tags.begin();
      double prob = alpha[words.size()%2][tag];
      /*if (prob <= 0 && debug) {
        cerr << <<"Problem with word '"<<word->get_superficial_form()<<"' "<<word->get_string_tags()<<"\n";
        }
       */
      uint64_t eof = 0; // tde.getTagIndex("TAG_kEOF"_u)
      for (uint64_t t = 0; t < best[words.size()%2][tag].size(); t++) {
        if (true) { // (TheFlags.getFirst()) {
          //write(words[t].get_all_chosen_tag_first(best[words.size()%2][tag][t],
          //                                        eof),
          //      output);
        } else {
          words[t].set_show_sf(false); // TheFlags.getShowSuperficial()
          //write(words[t].get_lexical_form(best[words.size()%2][tag][t], eof),
          //      output);
        }
      }
      words.clear();
      alpha[0][tag] = 1;
    }
    delete cur_word;
  }
}
