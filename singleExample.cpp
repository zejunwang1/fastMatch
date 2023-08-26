#include <fastMatch.h>

int main() {
  string disease_path = "data/disease.txt";
  FastMatch fastMatch(disease_path);
  string query = "乙肝大三阳抗病毒治疗需要多长时间？";
  // Single-pattern matching
  string pattern = "抗病毒治疗";
  int pos = match(query, pattern);
  if (pos >= 0)
    cout << "Find pattern at position: " << pos << endl;
  // Multi-pattern matching
  auto result = fastMatch.parse(query);
  cout << "\nMulti-pattern matching result:\n";
  for (int i = 0; i < result.size(); i++)
    cout << result[i].first << " " << result[i].second << endl;
  // Maximum forward matching word segmentation
  cout << "\nMaximum forward matching word segmentation result:\n";
  auto words = fastMatch.maxForwardMatch(query);
  for (auto& word : words)
    cout << word << " ";
  cout << endl;
  return 0;  
}

