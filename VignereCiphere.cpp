#include <bits/stdc++.h>
using namespace std;

// Global variables
string cipher_text;                // Processed ciphertext (letters only, uppercase)
string original_cipher_text;       // Original ciphertext from file
string plain_text;                 // Final decrypted plaintext with original formatting
string unprocessed_plain_text;     // Intermediate plaintext without formatting
vector<pair<int,int>> kasaki_voting(3);  // Voting mechanism for probable key lengths
vector<double> englishFrequencies = {0.08167, 0.01492, 0.02782, 0.04253, 0.12702, 0.02228, 
    0.02015, 0.06094, 0.06966, 0.00153, 0.00772, 0.04025, 0.02406, 0.06749, 0.07507, 
    0.01929, 0.00095, 0.05987, 0.06327, 0.09056, 0.02758, 0.00978, 0.02360, 0.00150, 
    0.01974, 0.00074};

// --------------------------- Function: preprocess ---------------------------
// Reads input.txt, removes non-alphabetic characters, converts to uppercase
void preprocess(){
    ifstream file("input.txt");
    if(!file){
        cerr << "Error: Can't open the file" << endl;
        return;
    }
    string line = "", refined_line = "";
    while(getline(file, line)){
        original_cipher_text += line;
        for(auto &it : line){
            if(isupper(it) || islower(it)){
                refined_line += toupper(it); // Convert lowercase to uppercase
            }
        }
        cipher_text += refined_line;
        refined_line.clear();
    }
}

// --------------------------- Function: best_factor ---------------------------
// Given differences between repeated patterns, votes for factors 3-5
int best_factor(vector<int> &differences){
    for(int factor = 3; factor <= 5; ++factor){
        int count = 0;
        for(int i = 0; i < differences.size(); ++i){
            if(differences[i] % factor == 0){
                count++;
            }
        }
        kasaki_voting[factor-3] = {factor, count};
    }
    // Sort by count descending, then by factor descending
    sort(kasaki_voting.begin(), kasaki_voting.end(), [](auto &a, auto &b){
        if(a.second == b.second)
            return a.first > b.first;
        return a.second > b.second;
    });
    return kasaki_voting[0].first;
}

// --------------------------- Function: gcd ---------------------------
// Compute GCD of two numbers
int gcd(int a, int b){
    while(a){
        int temp = a;
        a = b % a;
        b = temp;
    }
    return b;
}

// --------------------------- Function: gcd_all ---------------------------
// Compute GCD of all numbers in a vector
int gcd_all(vector<int> &differences){
    int n = differences.size();
    int g = differences[0];
    for(int i = 1; i < n; ++i){
        g = gcd(g, differences[i]);
    }
    return g;
}

// --------------------------- Function: kasaski_method ---------------------------
// Implements Kasiski examination to guess probable key length
int kasaski_method(string &cipher_text , int &possible_key_len){
    int n = cipher_text.size();
    unordered_map<string, vector<int>> mpp;

    // Check repeated substrings of length 3 to 5
    for(int len = 3; len <= 5; ++len){
        for(int i = 0; i < n - len + 1; ++i){
            string check = cipher_text.substr(i, len);
            string key = to_string(len) + "-" + check;
            mpp[key].push_back(i);
        }
    }

    vector<int> differences;
    for(auto &it : mpp){
        if(it.second.size() > 1){
            int n1 = it.second.size();
            for(int i = n1 - 1; i > 0; --i){
                for(int j = i - 1; j >= 0; --j){
                    int diff = it.second[i] - it.second[j];
                    if(diff)
                        differences.push_back(diff);
                }
            }
        }
    }

    if(differences.empty())
        return 1;

    possible_key_len = gcd_all(differences);

    // Fall back to voting mechanism if GCD = 1 , probabily induced due to noisy repeating substring
    if(possible_key_len == 1){
        possible_key_len = best_factor(differences);
    }
    return possible_key_len;
}

// --------------------------- Function: compute_ic ---------------------------
// Computes Index of Coincidence for a given subsequence
double compute_ic(string subsequence){
    int n = subsequence.size();
    if(n <= 1)
        return 0.0;

    vector<int> count(26);
    for(auto &it : subsequence)
        count[it - 'A']++;

    double total = 0;
    for(int i = 0; i < 26; ++i){
        total += (count[i] * (count[i] - 1));
    }

    return total / (n * (n - 1));
}

// --------------------------- Function: index_of_coincidence ---------------------------
// Confirms probable key length using Index of Coincidence
int index_of_coincidence(int &possible_key_len){
    vector<string> groups(possible_key_len);
    for(int i = 0; i < cipher_text.size(); ++i){
        groups[i % possible_key_len] += cipher_text[i];
    }

    vector<double> individual_ic(possible_key_len);
    for(int i = 0; i < possible_key_len; ++i){
        individual_ic[i] = compute_ic(groups[i]);
    }

    double total = accumulate(individual_ic.begin(), individual_ic.end(), 0.0) / individual_ic.size();

    if(total >= 0.06 && total <= 0.075)
        return possible_key_len;
    else 
        return 0;
}

// --------------------------- Function: compute_mic ---------------------------
// Computes mutual index of coincidence between two subsequences
double compute_mic(string &a , string &b){
    vector<int> freqA(26, 0);
    vector<int> freqB(26, 0);

    for(auto &it : a) freqA[it - 'A']++;
    for(auto &it : b) freqB[it - 'A']++;

    double mic = 0.0;
    for(int i = 0; i < 26; ++i){
        mic += (freqA[i] * freqB[i]);
    }
    mic /= (a.size() * b.size());
    return mic;
}

// --------------------------- Function: chi_square_test ---------------------------
// Chi-square test for English letter frequency
double chi_square_test(string &curr_plain_text){
    vector<double> count(26, 0);
    for(auto it : curr_plain_text) count[it - 'A']++;

    double chi = 0.0;
    int n = curr_plain_text.size();
    for(int i = 0; i < 26; ++i){
        double expected = n * englishFrequencies[i];
        if(expected > 0)
            chi += ((count[i] - expected) * (count[i] - expected)) / expected;
    }
    return chi;
}

// --------------------------- Function: decipher ---------------------------
// Performs Vigenère decryption using relative shifts and key length
int decipher(vector<int> &relative_shifts , int key_len){
    int n = cipher_text.size();
    double best_chi = 1e18;
    int best_key_shift = 0;

    for(int key_shifts = 0; key_shifts < 26; ++key_shifts){
        string curr_plain_text = cipher_text;
        vector<int> dummy_relative_shifts = relative_shifts;

        for(auto &it : dummy_relative_shifts)
            it = (it + key_shifts) % 26;

        for(int pos = 0; pos < n; ++pos){
            int idx = pos % key_len;
            char c = cipher_text[pos];
            char p = ((c - 'A' - dummy_relative_shifts[idx] + 26) % 26) + 'A';
            curr_plain_text[pos] = p;
        }

        double chi = chi_square_test(curr_plain_text);
        if(chi < best_chi){
            best_chi = chi;
            best_key_shift = key_shifts;
            unprocessed_plain_text = curr_plain_text;
        }
    }
    return best_key_shift;
}

// --------------------------- Function: mutual_index_of_coincidence ---------------------------
// Computes relative shifts between key letters using MIC
string mutual_index_of_coincidence(int key_len){
    vector<string> groups(key_len);
    for(int i = 0; i < cipher_text.size(); ++i){
        groups[i % key_len] += cipher_text[i];
    }

    string reference_subsequence = groups[0];
    vector<int> relative_shifts(key_len, 0);

    for(int idx = 1; idx < key_len; ++idx){
        int best_shift = 0;
        double best_mic = 0.0;

        for(int shift = 0; shift < 26; ++shift){
            string shifted_subsequence = "";
            for(int sub_idx = 0; sub_idx < groups[idx].size(); ++sub_idx){
                string curr_subsequence = groups[idx];
                shifted_subsequence += ((curr_subsequence[sub_idx] - 'A' - shift + 26) % 26) + 'A';
            }
            double mic = compute_mic(reference_subsequence, shifted_subsequence);
            if(mic > best_mic){
                best_mic = mic;
                best_shift = shift;
            }
        }
        relative_shifts[idx] = best_shift;
    }

    int best_key_shift = decipher(relative_shifts, key_len);

    string key = "";
    for(auto &it : relative_shifts){
        it = (it + best_key_shift) % 26;
        key += (it + 'A');
    }
    return key;
}

// --------------------------- Function: postprocess ---------------------------
// Restores original formatting (spaces, lowercase, punctuation) to plaintext
void postprocess(){
    int original_cipher_text_size = original_cipher_text.size();
    int unprocessed_plain_text_size = unprocessed_plain_text.size();
    int original_cipher_text_idx = 0;
    int unprocessed_plain_text_idx = 0;

    while(original_cipher_text_idx < original_cipher_text_size){
        if(isspace(original_cipher_text[original_cipher_text_idx])){
            plain_text += ' ';
            original_cipher_text_idx++;
        }
        else if(!islower(original_cipher_text[original_cipher_text_idx]) && !isupper(original_cipher_text[original_cipher_text_idx])){
            plain_text += original_cipher_text[original_cipher_text_idx];
            original_cipher_text_idx++;
        }
        else if(islower(original_cipher_text[original_cipher_text_idx]) && isupper(unprocessed_plain_text[unprocessed_plain_text_idx])){
            plain_text += tolower(unprocessed_plain_text[unprocessed_plain_text_idx]);
            original_cipher_text_idx++;
            unprocessed_plain_text_idx++;
        }
        else{
            plain_text += unprocessed_plain_text[unprocessed_plain_text_idx];
            original_cipher_text_idx++;
            unprocessed_plain_text_idx++;
        }
    }
}

// --------------------------- Function: file_output ---------------------------
// Writes key and plaintext to JSON file
void file_output(string& key, string& plainText, const string& filename) {
    ofstream file(filename);
    if (!file.is_open()) {
        cerr << "Error opening file for JSON output\n";
        return;
    }

    file << "{\n\n";
    file << "  \"key\": \"" << key << "\",\n\n";
    file << "  \"plaintext\": \"" << plainText << "\"\n\n";
    file << "}\n\n";

    file.close();
}

// --------------------------- MAIN FUNCTION ---------------------------
int main(){
    preprocess(); 

    int possible_key_len = 0;
    possible_key_len = kasaski_method(cipher_text, possible_key_len);
    cout << "[DEBUG] : Possible key length value\t: " << possible_key_len << endl;

    int confirmed_key_len = index_of_coincidence(possible_key_len);

    if(confirmed_key_len == 0){
        int i = 1;
        while(i < 3 && confirmed_key_len == 0){
            confirmed_key_len = index_of_coincidence(kasaki_voting[i].first);
            i++;
        }
    }

    if(confirmed_key_len == 0){
        cerr << "Error: Unable to determine key length from the ciphertext due to invalid key length or incompatible ciphertext." << endl;
        ofstream file("output.json");
        file << "{\n  \"error\": \"Unable to determine key length from the ciphertext due to invalid key length or incompatible ciphertext.\"\n}\n";
        file.close();
        return 1;
    }

    cout << "[DEBUG] : Confirmed key length value\t: " << confirmed_key_len << endl;

    string key = mutual_index_of_coincidence(confirmed_key_len);
    cout << "[DEBUG] : Key\t: " << key << endl;

    postprocess();
    file_output(key, plain_text, "output.json");

    return 0;
}
