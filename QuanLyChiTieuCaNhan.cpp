#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <iomanip>   
#include <cstdio>    
#include <cstdlib>   
#include <cstring>   

using namespace std;


constexpr int    MAX_CAT      = 50;
constexpr int    CAT_NAME_LEN = 30;
const string     TRANS_FILE   = "transactions.csv";
const string     BUDGET_FILE  = "budgets.csv";
const string     CAT_FILE     = "categories.csv";


// ===================================================================
// MODULE 1: CAC CAU TRUC DU LIEU
// ===================================================================

struct Category {
    int    id;
    string name;
    bool   active;   
};

struct Transaction {
    int       id;
    int       type;      // 0 = Chi, 1 = Thu
    int       category;  // ID danh muc
    long long amount;
    int       day, month, year;
    string    note;

    Transaction() : id(0), type(0), category(0), amount(0),
                    day(1), month(1), year(2025) {}
};

struct Budget {
    int       category;
    long long limit_val;
    int       month, year;

    Budget() : category(0), limit_val(0), month(1), year(2025) {}
};


// ===================================================================
// MODULE 2: MANG DONG TU VIET 
// ===================================================================

template<typename T>
class DynamicArray {
private:
    T*  data;
    int capacity;
    int sz;

    void grow() {
        capacity = (capacity > 0) ? capacity * 2 : 16;
        T* tmp = new T[capacity];
        for (int i = 0; i < sz; i++) tmp[i] = data[i];
        delete[] data;
        data = tmp;
    }

public:
    DynamicArray() : capacity(16), sz(0) {
        data = new T[capacity];
    }
    ~DynamicArray() { delete[] data; }

    DynamicArray(const DynamicArray&) = delete;
    DynamicArray& operator=(const DynamicArray&) = delete;

    void push_back(const T& t) {
        if (sz >= capacity) grow();
        data[sz++] = t;
    }

    int  size()  const { return sz; }
    T& at(int i) {
        if (i < 0 || i >= sz) {
            cerr << "[LOI NGHIEM TRONG] DynamicArray::at() - chi so " << i
                 << " vuot ngoai pham vi (0.." << sz-1 << ")\n";
            exit(1);
        }
        return data[i];
    }
    const T& at(int i) const {
        if (i < 0 || i >= sz) {
            cerr << "[LOI NGHIEM TRONG] DynamicArray::at() - chi so " << i
                 << " vuot ngoai pham vi (0.." << sz-1 << ")\n";
            exit(1);
        }
        return data[i];
    }

    void remove_at(int idx) {
        for (int i = idx; i < sz - 1; i++) data[i] = data[i + 1];
        sz--;
    }

    void clear() { sz = 0; }

    int find_by_id(int id) const {
        for (int i = 0; i < sz; i++)
            if (data[i].id == id) return i;
        return -1;
    }
};


// ===================================================================
// MODULE 3: BO NHO DEM TOAN CUC
// ===================================================================

DynamicArray<Category>    gCategories;
DynamicArray<Transaction> gTransactions;
DynamicArray<Budget>      gBudgets;


// ===================================================================
// MODULE 4: TIEN ICH
// ===================================================================

void clearInput() {
    cin.ignore(10000, '\n');
}

string formatMoney(long long amount) {
    bool neg = (amount < 0);
    if (neg) amount = -amount;

    string digits = to_string(amount);
    string result;
    int len = (int)digits.size();
    int rem = len % 3;

    for (int i = 0; i < len; i++) {
        if (i > 0 && (i - rem) % 3 == 0 && i >= rem)
            result += '.';
        result += digits[i];
    }
    return (neg ? "-" : "") + result;
}

// Kiem tra nam nhuan
bool isLeapYear(int y) {
    return (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0);
}

// Tra ve so ngay trong thang
int daysInMonth(int m, int y) {
    int days[] = {0,31,28,31,30,31,30,31,31,30,31,30,31};
    if (m == 2 && isLeapYear(y)) return 29;
    return (m >= 1 && m <= 12) ? days[m] : 0;
}

// Kiem tra ngay hop le
bool isValidDate(int d, int m, int y) {
    if (y < 1900 || y > 2100) return false;
    if (m < 1 || m > 12)      return false;
    if (d < 1 || d > daysInMonth(m, y)) return false;
    return true;
}

// Nhap so nguyen co kiem tra hop le
int inputInt(const string& prompt) {
    int val;
    while (true) {
        cout << prompt;
        if (cin >> val) { clearInput(); return val; }
        cin.clear();
        clearInput();
        cout << "  [!] Vui long nhap so nguyen hop le!\n";
    }
}

// Nhap so tien duong
long long inputMoney(const string& prompt) {
    long long val;
    while (true) {
        cout << prompt;
        if (cin >> val && val > 0) { clearInput(); return val; }
        cin.clear();
        clearInput();
        cout << "  [!] So tien phai la so nguyen duong (> 0)!\n";
    }
}

// Nhap ngay thang nam co validate day du
void inputDate(int& day, int& month, int& year) {
    while (true) {
        year = inputInt("  Nam  (vd 2025) : ");
        if (year < 1900 || year > 2100) {
            cout << "  [!] Nam phai tu 1900 den 2100!\n"; continue;
        }
        month = inputInt("  Thang (1-12)   : ");
        if (month < 1 || month > 12) {
            cout << "  [!] Thang phai tu 1 den 12!\n"; continue;
        }
        int maxDay = daysInMonth(month, year);
        cout << "  (Thang " << month << "/" << year
             << " co " << maxDay << " ngay)\n";
        day = inputInt("  Ngay            : ");
        if (day < 1 || day > maxDay) {
            cout << "  [!] Ngay phai tu 1 den " << maxDay
                 << " trong thang " << month << "/" << year << "!\n";
            continue;
        }
        break;
    }
}

// Ham tim kiem chuoi con
// Lay tu ca hai file, dung cho tim kiem ghi chu
bool strContainsCI(const string& text, const string& kw) {
    if (kw.empty()) return true;
    if (text.size() < kw.size()) return false;

    // Chuyen ve chu thuong de so sanh
    string t2 = text, k2 = kw;
    for (char& c : t2) if (c >= 'A' && c <= 'Z') c += 32;
    for (char& c : k2) if (c >= 'A' && c <= 'Z') c += 32;

    for (size_t i = 0; i <= t2.size() - k2.size(); i++) {
        size_t j = 0;
        while (j < k2.size() && t2[i+j] == k2[j]) j++;
        if (j == k2.size()) return true;
    }
    return false;
}

// Trim dau va cuoi chuoi
string trim(const string& s) {
    int start = 0, end = (int)s.size() - 1;
    while (start <= end && (s[start]==' '||s[start]=='\t'||
                             s[start]=='\r'||s[start]=='\n')) start++;
    while (end >= start && (s[end]==' '||s[end]=='\t'||
                             s[end]=='\r'||s[end]=='\n')) end--;
    return s.substr(start, end - start + 1);
}

// Tach chuoi CSV theo dau phay
// Tra ve mang cac token
void splitCSV(const string& line, string tokens[], int& count, int maxTok) {
    count = 0;
    if (maxTok <= 0) return;
    string cur;
    for (size_t i = 0; i < line.size(); i++) {
        if (line[i] == ',') {
            if (count < maxTok - 1) {
                tokens[count++] = cur;
                cur.clear();
            } else {
                // Da du so truong, phan con lai gop vao token cuoi
                cur += line[i];
            }
        } else {
            cur += line[i];
        }
    }
    // Ghi token cuoi
    if (count < maxTok) {
        tokens[count++] = cur;
    }
}

// Xoa ky tu dau phay trong chuoi
string sanitizeForCSV(const string& s) {
    string out;
    for (char c : s) out += (c == ',') ? '.' : c;
    return out;
}


// ===================================================================
// MODULE 5: QUAN LY DANH MUC
// ===================================================================

// Lay ID lon nhat hien co trong danh sach danh muc
static int getMaxCatID() {
    int mx = 0;
    for (int i = 0; i < gCategories.size(); i++)
        if (gCategories.at(i).id > mx) mx = gCategories.at(i).id;
    return mx;
}

// Kiem tra ten danh muc da ton tai chua
static bool catNameExists(const string& name, int excludeID = -1) {
    for (int i = 0; i < gCategories.size(); i++) {
        const Category& c = gCategories.at(i);
        if (!c.active) continue;
        if (c.id == excludeID) continue;
        if (strContainsCI(c.name, name) && strContainsCI(name, c.name))
            return true;  // ten giong nhau (khong phan biet hoa/thuong)
    }
    return false;
}

// Lay ten danh muc theo ID
string getCategoryName(int id) {
    for (int i = 0; i < gCategories.size(); i++) {
        const Category& c = gCategories.at(i);
        if (c.id == id && c.active) return c.name;
    }
    return "(Da xoa)";
}

// Tim chi so trong gCategories theo ID
int findCatIndex(int id) {
    for (int i = 0; i < gCategories.size(); i++)
        if (gCategories.at(i).id == id) return i;
    return -1;
}

// Khoi tao 8 danh muc mac dinh khi chua co file
void initDefaultCategories() {
    const string defs[] = {
        "An uong","Di lai","Hoc phi","Mua sam",
        "Giai tri","Y te","Luong","Khac"
    };
    for (int i = 0; i < 8; i++) {
        Category c;
        c.id     = i + 1;
        c.name   = defs[i];
        c.active = true;
        gCategories.push_back(c);
    }
}

// In danh sach danh muc dang hoat dong
void printCategories() {
    cout << "\n  +--- DANH MUC HIEN CO ---+\n";
    cout << "  " << string(29, '-') << "\n";
    cout << "  " << left << setw(6) << "ID" << "Ten danh muc\n";
    cout << "  " << string(29, '-') << "\n";
    for (int i = 0; i < gCategories.size(); i++) {
        const Category& c = gCategories.at(i);
        if (c.active)
            cout << "  " << left << setw(6) << c.id << c.name << "\n";
    }
    cout << "  " << string(29, '-') << "\n";
}

// Nhap ID danh muc hop le tu nguoi dung
int inputCategory() {
    printCategories();
    while (true) {
        int id = inputInt("  Chon ID danh muc: ");
        int idx = findCatIndex(id);
        if (idx >= 0 && gCategories.at(idx).active) return id;
        cout << "  [!] ID " << id
             << " khong ton tai hoac da bi xoa. Chon lai!\n";
    }
}

bool saveCategories() {
    string tmpFile = CAT_FILE + ".tmp";
    ofstream fp(tmpFile);
    if (!fp) {
        cout << "  [Loi] Khong the ghi file " << CAT_FILE << "!\n";
        return false;
    }
    fp << "// id,active,name\n";
    for (int i = 0; i < gCategories.size(); i++) {
        const Category& c = gCategories.at(i);
        fp << c.id << "," << (c.active ? 1 : 0) << ","
           << sanitizeForCSV(c.name) << "\n";
    }
    fp.close();
    if (remove(CAT_FILE.c_str()) != 0) {
        // Bo qua neu file khong ton tai
    }
    if (rename(tmpFile.c_str(), CAT_FILE.c_str()) != 0) {
        cout << "  [Loi] Khong the luu file " << CAT_FILE << "!\n";
        remove(tmpFile.c_str());
        return false;
    }
    return true;
}

// Doc danh muc tu file CSV
void loadCategories() {
    gCategories.clear();
    ifstream fp(CAT_FILE.c_str());
    if (!fp) {
        initDefaultCategories();
        return;
    }
    string line;
    bool loaded = false;
    while (getline(fp, line)) {
        line = trim(line);
        if (line.empty() || (line.size()>=2 && line[0]=='/' && line[1]=='/'))
            continue;
        try {
            string tok[4];
            int cnt = 0;
            splitCSV(line, tok, cnt, 4);
            if (cnt < 3) continue;
            Category c;
            c.id     = stoi(tok[0]);
            c.active = (stoi(tok[1]) != 0);
            c.name   = trim(tok[2]);
            if (c.name.empty()) continue;
            gCategories.push_back(c);
            loaded = true;
        } catch (...) { continue; }
    }
    if (!loaded) initDefaultCategories();
}

// Menu con quan ly danh muc
void categoryMenu() {
    int sub;
    do {
        cout << "\n  +------ QUAN LY DANH MUC ------+\n";
        cout << "  1. Xem danh sach danh muc\n";
        cout << "  2. Them danh muc moi\n";
        cout << "  3. Sua ten danh muc\n";
        cout << "  4. Xoa danh muc\n";
        cout << "  0. Quay lai menu chinh\n";
        sub = inputInt("  Chon (0-4): ");

        if (sub == 1) {
            printCategories();
            bool hasDeleted = false;
            for (int i = 0; i < gCategories.size(); i++) {
                if (!gCategories.at(i).active) {
                    if (!hasDeleted) {
                        cout << "\n  Danh muc da xoa mem:\n";
                        hasDeleted = true;
                    }
                    cout << "  [ID=" << gCategories.at(i).id << "] "
                         << gCategories.at(i).name << "\n";
                }
            }
            if (!hasDeleted) cout << "  (Khong co danh muc da xoa)\n";

        } else if (sub == 2) {
            if (gCategories.size() >= MAX_CAT) {
                cout << "  [!] Danh sach danh muc da day (" << MAX_CAT << ")!\n";
            } else {
                cout << "  Nhap ten danh muc moi: ";
                string name; getline(cin, name);
                name = trim(name);
                name = sanitizeForCSV(name);
                if (name.empty()) {
                    cout << "  [!] Ten khong duoc de trong!\n";
                } else if (catNameExists(name)) {
                    cout << "  [!] Ten \"" << name << "\" da ton tai!\n";
                } else {
                    Category c;
                    c.id     = getMaxCatID() + 1;
                    c.name   = name;
                    c.active = true;
                    gCategories.push_back(c);
                    cout << "  >> Da them danh muc [ID=" << c.id
                         << "] \"" << name << "\"\n";
                    saveCategories();
                }
            }

        } else if (sub == 3) {
            printCategories();
            int id = inputInt("  Nhap ID danh muc can sua: ");
            int idx = findCatIndex(id);
            if (idx < 0 || !gCategories.at(idx).active) {
                cout << "  [!] Khong tim thay danh muc ID=" << id << "!\n";
            } else {
                cout << "  Ten hien tai: \"" << gCategories.at(idx).name << "\"\n";
                cout << "  Ten moi     : ";
                string name; getline(cin, name);
                name = trim(name);
                name = sanitizeForCSV(name);
                if (name.empty()) {
                    cout << "  [!] Ten khong duoc de trong!\n";
                } else if (catNameExists(name, id)) {
                    cout << "  [!] Ten \"" << name << "\" da ton tai!\n";
                } else {
                    cout << "  >> Da doi \"" << gCategories.at(idx).name
                         << "\" -> \"" << name << "\"\n";
                    gCategories.at(idx).name = name;
                    saveCategories();
                }
            }

        } else if (sub == 4) {
            printCategories();
            int id = inputInt("  Nhap ID danh muc can xoa: ");
            int idx = findCatIndex(id);
            if (idx < 0 || !gCategories.at(idx).active) {
                cout << "  [!] Khong tim thay danh muc ID=" << id << "!\n";
            } else {
                // Dem so giao dich dang dung danh muc nay
                int used = 0;
                for (int i = 0; i < gTransactions.size(); i++)
                    if (gTransactions.at(i).category == id) used++;

                if (used > 0) {
                    cout << "  [!] Danh muc \"" << gCategories.at(idx).name
                         << "\" dang duoc dung boi " << used << " giao dich!\n";
                    cout << "  Cac giao dich se hien thi '(Da xoa)'. Tiep tuc? (y/n): ";
                    string _confirm_cat1; getline(cin, _confirm_cat1);
                    char c = _confirm_cat1.empty() ? 'n' : _confirm_cat1[0];
                    if (c != 'y' && c != 'Y') {
                        cout << "  >> Huy xoa.\n";
                        continue;
                    }
                }
                cout << "  Xac nhan xoa danh muc \""
                     << gCategories.at(idx).name << "\"? (y/n): ";
                string _confirm_cat2; getline(cin, _confirm_cat2);
                char c = _confirm_cat2.empty() ? 'n' : _confirm_cat2[0];
                if (c == 'y' || c == 'Y') {
                    cout << "  >> Da xoa danh muc [ID=" << id << "] \""
                         << gCategories.at(idx).name << "\"\n";
                    gCategories.at(idx).active = false;
                    saveCategories();
                } else {
                    cout << "  >> Huy xoa.\n";
                }
            }
        } else if (sub != 0) {
            cout << "  [!] Lua chon khong hop le!\n";
        }
    } while (sub != 0);
}


// ===================================================================
// MODULE 6: DOC / GHI FILE GIAO DICH & NGAN SACH
// ===================================================================

void loadTransactions() {
    gTransactions.clear();
    ifstream fp(TRANS_FILE.c_str());
    if (!fp) return;
    string line;
    while (getline(fp, line)) {
        line = trim(line);
        if (line.empty() || (line.size()>=2 && line[0]=='/' && line[1]=='/'))
            continue;
        try {
            string tok[9];
            int cnt = 0;
            splitCSV(line, tok, cnt, 9);
            if (cnt < 8) continue;
            Transaction t;
            t.id       = stoi(tok[0]);
            t.type     = stoi(tok[1]);
            t.category = stoi(tok[2]);
            t.amount   = stoll(tok[3]);
            t.day      = stoi(tok[4]);
            t.month    = stoi(tok[5]);
            t.year     = stoi(tok[6]);
            t.note     = trim(tok[7]);
            // Kiem tra tinh hop le co ban truoc khi nap vao bo nho
            if (!isValidDate(t.day, t.month, t.year)) continue;
            if (t.amount <= 0) continue;
            if (t.type < 0 || t.type > 1) continue;
            gTransactions.push_back(t);
        } catch (...) { continue; }
    }
}

void saveTransactions() {
    string tmpFile = TRANS_FILE + ".tmp";
    ofstream fp(tmpFile);
    if (!fp) { cout << "  [Loi] Khong the ghi file " << TRANS_FILE << "!\n"; return; }
    fp << "// id,type,category,amount,day,month,year,note\n";
    for (int i = 0; i < gTransactions.size(); i++) {
        const Transaction& t = gTransactions.at(i);
        fp << t.id << "," << t.type << "," << t.category << ","
           << t.amount << "," << t.day << "," << t.month << ","
           << t.year << "," << sanitizeForCSV(t.note) << "\n";
    }
    fp.close();
    if (remove(TRANS_FILE.c_str()) != 0) {
        // Bo qua neu file khong ton tai
    }
    if (rename(tmpFile.c_str(), TRANS_FILE.c_str()) != 0) {
        cout << "  [Loi] Khong the luu file " << TRANS_FILE << "!\n";
        remove(tmpFile.c_str());
    }
}

void loadBudgets() {
    gBudgets.clear();
    ifstream fp(BUDGET_FILE.c_str());
    if (!fp) return;
    string line;
    while (getline(fp, line)) {
        line = trim(line);
        if (line.empty() || (line.size()>=2 && line[0]=='/' && line[1]=='/'))
            continue;
        try {
            string tok[5];
            int cnt = 0;
            splitCSV(line, tok, cnt, 5);
            if (cnt < 4) continue;
            Budget b;
            b.category  = stoi(tok[0]);
            b.limit_val = stoll(tok[1]);
            b.month     = stoi(tok[2]);
            b.year      = stoi(tok[3]);
            if (b.limit_val <= 0) continue;
            gBudgets.push_back(b);
        } catch (...) { continue; }
    }
}

void saveBudgets() {
    string tmpFile = BUDGET_FILE + ".tmp";
    ofstream fp(tmpFile);
    if (!fp) { cout << "  [Loi] Khong the ghi file " << BUDGET_FILE << "!\n"; return; }
    fp << "// category,limit,month,year\n";
    for (int i = 0; i < gBudgets.size(); i++) {
        const Budget& b = gBudgets.at(i);
        fp << b.category << "," << b.limit_val << ","
           << b.month << "," << b.year << "\n";
    }
    fp.close();
    if (remove(BUDGET_FILE.c_str()) != 0) {
        // Bo qua neu file khong ton tai
    }
    if (rename(tmpFile.c_str(), BUDGET_FILE.c_str()) != 0) {
        cout << "  [Loi] Khong the luu file " << BUDGET_FILE << "!\n";
        remove(tmpFile.c_str());
    }
}

// Luu tat ca file trong 1 lan goi
void saveAll() {
    saveTransactions();
    saveBudgets();
    saveCategories();
}


// ===================================================================
// MODULE 7: LOGIC NGHIEP VU NGAN SACH
// ===================================================================

// Tinh tong chi cua 1 danh muc trong 1 thang/nam
long long getTotalSpent(int catID, int month, int year) {
    long long total = 0;
    for (int i = 0; i < gTransactions.size(); i++) {
        const Transaction& t = gTransactions.at(i);
        if (t.type == 0 && t.category == catID &&
            t.month == month && t.year == year)
            total += t.amount;
    }
    return total;
}

// Kiem tra xem giao dich moi co vuot budget khong 
bool isBudgetExceeded(int catID, long long newAmt, int month, int year,
                      long long& limitOut) {
    for (int i = 0; i < gBudgets.size(); i++) {
        const Budget& b = gBudgets.at(i);
        if (b.category == catID && b.month == month && b.year == year) {
            limitOut = b.limit_val;
            return (getTotalSpent(catID, month, year) + newAmt) > b.limit_val;
        }
    }
    return false;
}

// Sinh ID moi cho giao dich
int getNextTransID() {
    int mx = 0;
    for (int i = 0; i < gTransactions.size(); i++)
        if (gTransactions.at(i).id > mx) mx = gTransactions.at(i).id;
    return mx + 1;
}


// ===================================================================
// MODULE 8: CAC TINH NANG GIAO DICH
// ===================================================================

// 1. Them giao dich
void addTransaction() {
    cout << "\n+------ THEM GIAO DICH MOI ------+\n";
    Transaction t;
    t.id = getNextTransID();

    // Nhap loai
    while (true) {
        int v = inputInt("  Loai giao dich (0=Chi, 1=Thu): ");
        if (v == 0 || v == 1) { t.type = v; break; }
        cout << "  [!] Chi nhap 0 hoac 1!\n";
    }

    // Nhap danh muc
    t.category = inputCategory();

    // Nhap so tien
    t.amount = inputMoney("  So tien (VND): ");

    // Nhap ngay
    cout << "  Nhap ngay giao dich:\n";
    inputDate(t.day, t.month, t.year);

    // Nhap ghi chu
    cout << "  Ghi chu: ";
    getline(cin, t.note);
    t.note = trim(sanitizeForCSV(t.note));

    // Canh bao vuot budget REAL-TIME
    if (t.type == 0) {
        long long limit = 0;
        if (isBudgetExceeded(t.category, t.amount, t.month, t.year, limit)) {
            long long spent = getTotalSpent(t.category, t.month, t.year);
            cout << "\n  [!!!] CANH BAO: Khoan chi nay se VUOT HAN MUC BUDGET!\n";
            cout << "  Danh muc  : " << getCategoryName(t.category) << "\n";
            cout << "  Han muc   : " << formatMoney(limit) << " VND\n";
            cout << "  Da chi    : " << formatMoney(spent) << " VND\n";
            cout << "  Them vao  : " << formatMoney(t.amount) << " VND\n";
            cout << "  Se la     : " << formatMoney(spent + t.amount)
                 << " VND (vuot " << formatMoney(spent + t.amount - limit) << " VND)\n";
            cout << "  Tiep tuc ghi lai? (y/n): ";
            string _confirm_add; getline(cin, _confirm_add);
            char c = _confirm_add.empty() ? 'n' : _confirm_add[0];
            if (c != 'y' && c != 'Y') {
                cout << "  >> Da huy them giao dich.\n";
                return;
            }
        }
    }

    gTransactions.push_back(t);
    saveAll();
    cout << "  >> Da them ID=" << t.id
         << " | " << (t.type ? "Thu" : "Chi")
         << " | " << formatMoney(t.amount) << " VND"
         << " | " << getCategoryName(t.category)
         << " | " << t.day << "/" << t.month << "/" << t.year << "\n";
}

// 2. Xem danh sach giao dich
void listTransactions() {
    cout << "\n+------ XEM DANH SACH GIAO DICH ------+\n";
    cout << "  Loc theo:\n";
    cout << "    1. Tat ca\n";
    cout << "    2. Theo thang/nam\n";
    cout << "    3. Theo danh muc\n";
    cout << "    4. Theo ngay cu the\n";
    int choice = inputInt("  Chon (1-4): ");

    int fDay=0, fMonth=0, fYear=0, fCat=0;
    if (choice == 2) {
        fMonth = inputInt("  Thang (1-12): ");
        fYear  = inputInt("  Nam         : ");
    } else if (choice == 3) {
        fCat = inputCategory();
    } else if (choice == 4) {
        cout << "  Nhap ngay can loc:\n";
        inputDate(fDay, fMonth, fYear);
    }

    // Header bang
    cout << "\n  "
         << left  << setw(5)  << "ID"
         << setw(6)  << "Loai"
         << setw(15) << "Danh muc"
         << right << setw(19) << "So tien (VND)"
         << "  " << left << setw(13) << "Ngay"
         << "Ghi chu\n";
    cout << "  " << string(78, '-') << "\n";

    int n = 0;
    long long tongThu = 0, tongChi = 0;
    for (int i = 0; i < gTransactions.size(); i++) {
        const Transaction& t = gTransactions.at(i);
        if (choice == 2 && (t.month != fMonth || t.year != fYear)) continue;
        if (choice == 3 && t.category != fCat) continue;
        if (choice == 4 && (t.day!=fDay||t.month!=fMonth||t.year!=fYear)) continue;

        // Tao chuoi ngay theo dinh dang dd/mm/yyyy
        ostringstream dateStr;
        dateStr << setfill('0')
                << setw(2) << t.day << "/"
                << setw(2) << t.month << "/"
                << setw(4) << t.year;
        cout << "  "
             << left  << setw(5)  << t.id
             << setw(6)  << (t.type ? "Thu" : "Chi")
             << setw(15) << getCategoryName(t.category)
             << right << setw(19) << formatMoney(t.amount)
             << "  " << left << setw(13) << dateStr.str()
             << t.note << "\n";
        if (t.type == 1) tongThu += t.amount;
        else             tongChi += t.amount;
        n++;
    }
    cout << "  " << string(78, '-') << "\n";
    if (n == 0) { cout << "  (Khong co giao dich nao phu hop)\n"; return; }
    cout << "  Tong: " << n << " giao dich"
         << " | Thu: "    << formatMoney(tongThu) << " VND"
         << " | Chi: "    << formatMoney(tongChi) << " VND"
         << " | Can doi: "<< formatMoney(tongThu - tongChi) << " VND\n";
}

// 3. Xem so du hien tai
void showBalance() {
    long long thu = 0, chi = 0;
    for (int i = 0; i < gTransactions.size(); i++) {
        const Transaction& t = gTransactions.at(i);
        if (t.type == 1) thu += t.amount;
        else             chi += t.amount;
    }
    cout << "\n+------ SO DU HIEN TAI ------+\n";
    cout << "  Tong thu : " << formatMoney(thu) << " VND\n";
    cout << "  Tong chi : " << formatMoney(chi) << " VND\n";
    cout << "  So du    : " << formatMoney(thu - chi) << " VND\n";
}

// 8. Tim kiem giao dich
void searchTransactions() {
    cout << "\n+------ TIM KIEM GIAO DICH ------+\n";
    cout << "  Nhap tu khoa (trong ghi chu, khong phan biet hoa/thuong): ";
    string kw; getline(cin, kw);
    kw = trim(kw);

    cout << "\n  "
         << left  << setw(5)  << "ID"
         << setw(6)  << "Loai"
         << setw(15) << "Danh muc"
         << right << setw(19) << "So tien (VND)"
         << "  " << left << setw(13) << "Ngay"
         << "Ghi chu\n";
    cout << "  " << string(78, '-') << "\n";

    int found = 0;
    for (int i = 0; i < gTransactions.size(); i++) {
        const Transaction& t = gTransactions.at(i);
        if (strContainsCI(t.note, kw)) {
            ostringstream dateStr;
            dateStr << setfill('0')
                    << setw(2) << t.day << "/"
                    << setw(2) << t.month << "/"
                    << setw(4) << t.year;
            cout << "  "
                 << left  << setw(5)  << t.id
                 << setw(6)  << (t.type ? "Thu" : "Chi")
                 << setw(15) << getCategoryName(t.category)
                 << right << setw(19) << formatMoney(t.amount)
                 << "  " << left << setw(13) << dateStr.str()
                 << t.note << "\n";
            found++;
        }
    }
    cout << "  " << string(78, '-') << "\n";
    if (found == 0)
        cout << "  Khong tim thay ket qua nao voi tu khoa \"" << kw << "\".\n";
    else
        cout << "  Tim thay " << found << " ket qua.\n";
}

// 9. Xoa giao dich
void deleteTransaction() {
    cout << "\n+------ XOA GIAO DICH ------+\n";
    int id = inputInt("  Nhap ID can xoa: ");
    int idx = gTransactions.find_by_id(id);
    if (idx == -1) {
        cout << "  [!] Khong tim thay giao dich ID=" << id << "\n";
        return;
    }
    const Transaction& t = gTransactions.at(idx);
    cout << "  Giao dich: [ID=" << t.id << "] "
         << (t.type ? "Thu" : "Chi") << " | "
         << getCategoryName(t.category) << " | "
         << formatMoney(t.amount) << " VND | "
         << t.day << "/" << t.month << "/" << t.year
         << " | " << t.note << "\n";
    cout << "  Xac nhan xoa? (y/n): ";
    string _confirm_del; getline(cin, _confirm_del);
    char c = _confirm_del.empty() ? 'n' : _confirm_del[0];
    if (c == 'y' || c == 'Y') {
        gTransactions.remove_at(idx);
        saveAll();
        cout << "  >> Da xoa giao dich ID=" << id << "\n";
    } else {
        cout << "  >> Huy xoa.\n";
    }
}


// ===================================================================
// MODULE 9: THONG KE
// ===================================================================

// 4a. Thong ke theo thang
void calcByMonth() {
    int month = inputInt("  Nhap thang (1-12): ");
    int year  = inputInt("  Nhap nam         : ");
    long long thu = 0, chi = 0;
    for (int i = 0; i < gTransactions.size(); i++) {
        const Transaction& t = gTransactions.at(i);
        if (t.month != month || t.year != year) continue;
        if (t.type == 1) thu += t.amount; else chi += t.amount;
    }
    cout << "\n  Thang " << setfill('0') << setw(2) << month
         << "/" << setfill(' ') << year << ":\n";
    cout << "    Tong thu : " << formatMoney(thu) << " VND\n";
    cout << "    Tong chi : " << formatMoney(chi) << " VND\n";
    cout << "    Can doi  : " << formatMoney(thu - chi) << " VND\n";
}

// 4b. Thong ke theo nam
void calcByYear() {
    int year = inputInt("  Nhap nam: ");
    long long thu = 0, chi = 0;
    long long thuM[13] = {}, chiM[13] = {};
    for (int i = 0; i < gTransactions.size(); i++) {
        const Transaction& t = gTransactions.at(i);
        if (t.year != year) continue;
        int m = (t.month >= 1 && t.month <= 12) ? t.month : 0;
        if (t.type == 1) { thu += t.amount; if(m) thuM[m] += t.amount; }
        else             { chi += t.amount; if(m) chiM[m] += t.amount; }
    }
    cout << "\n  Thong ke nam " << year << ":\n";
    cout << "  " << left  << setw(11) << "Thang"
         << right << setw(21) << "Thu (VND)"
         << setw(21) << "Chi (VND)"
         << setw(21) << "Can doi (VND)" << "\n";
    cout << "  " << string(74, '-') << "\n";
    for (int m = 1; m <= 12; m++) {
        if (thuM[m] == 0 && chiM[m] == 0) continue;
        cout << "  " << left  << setw(11) << ("Thang " + to_string(m))
             << right << setw(21) << formatMoney(thuM[m])
             << setw(21) << formatMoney(chiM[m])
             << setw(21) << formatMoney(thuM[m] - chiM[m]) << "\n";
    }
    cout << "  " << string(74, '-') << "\n";
    cout << "  " << left  << setw(11) << "Tong"
         << right << setw(21) << formatMoney(thu)
         << setw(21) << formatMoney(chi)
         << setw(21) << formatMoney(thu - chi) << "\n";
}

// 4c. Thong ke theo danh muc trong thang
void calcByCategory() {
    int month = inputInt("  Nhap thang (1-12): ");
    int year  = inputInt("  Nhap nam         : ");

    // Dung map don gian: catID -> (thu, chi)
    // Vi khong co <map>, dung mang phu
    // catID co the len cao, dung mang tren gCategories
    const int MAXC = 512;  // an toan du cho moi truong hop
    long long chiC[MAXC] = {}, thuC[MAXC] = {};

    for (int i = 0; i < gTransactions.size(); i++) {
        const Transaction& t = gTransactions.at(i);
        if (t.month != month || t.year != year) continue;
        if (t.category < 0 || t.category >= MAXC) continue;
        if (t.type == 1) thuC[t.category] += t.amount;
        else             chiC[t.category] += t.amount;
    }

    cout << "\n  Thang " << setfill('0') << setw(2) << month
         << "/" << setfill(' ') << year << " theo danh muc:\n";
    cout << "  " << left  << setw(17) << "Danh muc"
         << right << setw(21) << "Chi (VND)"
         << setw(21) << "Thu (VND)" << "\n";
    cout << "  " << string(59, '-') << "\n";
    bool any = false;
    for (int i = 0; i < gCategories.size(); i++) {
        const Category& c = gCategories.at(i);
        if (c.id < 0 || c.id >= MAXC) continue;
        if (chiC[c.id] == 0 && thuC[c.id] == 0) continue;
        cout << "  " << left  << setw(17) << c.name
             << right << setw(21) << formatMoney(chiC[c.id])
             << setw(21) << formatMoney(thuC[c.id]) << "\n";
        any = true;
    }
    if (!any) cout << "  (Khong co giao dich nao trong thang nay)\n";
}

// 4d. Menu thong ke
void statsMenu() {
    cout << "\n+------ THONG KE THEO KY ------+\n";
    cout << "  1. Thong ke theo thang\n";
    cout << "  2. Thong ke theo nam\n";
    cout << "  3. Thong ke theo danh muc (trong thang)\n";
    int sub = inputInt("  Chon (1-3): ");
    switch (sub) {
        case 1: calcByMonth();    break;
        case 2: calcByYear();     break;
        case 3: calcByCategory(); break;
        default: cout << "  Lua chon khong hop le.\n";
    }
}

// 7. Bieu do chi tieu text
void printChart() {
    int month = inputInt("  Nhap thang (1-12): ");
    int year  = inputInt("  Nhap nam         : ");

    const int MAXC = 512;
    long long chiCat[MAXC] = {};
    long long total = 0;
    for (int i = 0; i < gTransactions.size(); i++) {
        const Transaction& t = gTransactions.at(i);
        if (t.month != month || t.year != year || t.type != 0) continue;
        if (t.category < 0 || t.category >= MAXC) continue;
        chiCat[t.category] += t.amount;
        total += t.amount;
    }
    if (total == 0) {
        cout << "  Khong co chi tieu nao trong thang "
             << setfill('0') << setw(2) << month
             << "/" << setfill(' ') << year << ".\n";
        return;
    }

    long long maxVal = 0;
    for (int i = 0; i < MAXC; i++)
        if (chiCat[i] > maxVal) maxVal = chiCat[i];

    cout << "\n  Bieu do chi tieu thang "
         << setfill('0') << setw(2) << month
         << "/" << setfill(' ') << year << ":\n";
    cout << "  " << string(60, '=') << "\n";
    const int BAR = 30;
    for (int i = 0; i < gCategories.size(); i++) {
        const Category& c = gCategories.at(i);
        if (c.id < 0 || c.id >= MAXC || chiCat[c.id] == 0) continue;
        int barLen = (maxVal > 0) ? (int)((double)chiCat[c.id] / maxVal * BAR) : 0;
        double pct = (double)chiCat[c.id] * 100.0 / total;
        // In ten danh muc can phai (12 ky tu)
        string nm = c.name;
        if ((int)nm.size() > 12) nm = nm.substr(0, 12);
        cout << "  " << left << setw(12) << nm << "|";
        for (int j = 0; j < barLen; j++) cout << "#";
        // In phan tram voi 1 chu so thap phan
        ostringstream pctStr;
        pctStr << fixed;
        pctStr.precision(1);
        pctStr << pct;
        cout << " " << pctStr.str() << "% ("
             << formatMoney(chiCat[c.id]) << " VND)\n";
    }
    cout << "  " << string(60, '=') << "\n";
    cout << "  Tong chi: " << formatMoney(total) << " VND\n";
}


// ===================================================================
// MODULE 10: QUAN LY NGAN SACH
// ===================================================================

static void listBudgets() {
    if (gBudgets.size() == 0) { cout << "  Chua co ngan sach nao.\n"; return; }
    cout << "\n  " << left  << setw(17) << "Danh muc"
         << setw(9)  << "Thang"
         << setw(7)  << "Nam"
         << right << setw(21) << "Han muc (VND)" << "\n";
    cout << "  " << string(54, '-') << "\n";
    for (int i = 0; i < gBudgets.size(); i++) {
        const Budget& b = gBudgets.at(i);
        cout << "  " << left  << setw(17) << getCategoryName(b.category)
             << setw(9)  << b.month
             << setw(7)  << b.year
             << right << setw(21) << formatMoney(b.limit_val) << "\n";
    }
}

// Dat hoac cap nhat ngan sach
static void setBudget() {
    cout << "\n  Dat/Cap nhat han muc budget:\n";
    int cat   = inputCategory();
    int month = inputInt("  Thang (1-12): ");
    if (month < 1 || month > 12) { cout << "  [!] Thang khong hop le!\n"; return; }
    int year  = inputInt("  Nam         : ");
    if (year < 1900 || year > 2100) { cout << "  [!] Nam khong hop le!\n"; return; }
    long long lim = inputMoney("  Han muc chi (VND): ");

    for (int i = 0; i < gBudgets.size(); i++) {
        Budget& b = gBudgets.at(i);
        if (b.category == cat && b.month == month && b.year == year) {
            cout << "  >> Cap nhat " << getCategoryName(cat)
                 << " thang " << month << "/" << year << ": "
                 << formatMoney(b.limit_val) << " -> "
                 << formatMoney(lim) << " VND\n";
            b.limit_val = lim;
            saveBudgets();
            return;
        }
    }
    Budget b;
    b.category = cat; b.limit_val = lim;
    b.month = month;  b.year = year;
    gBudgets.push_back(b);
    cout << "  >> Da dat han muc " << getCategoryName(cat)
         << " thang " << month << "/" << year << ": "
         << formatMoney(lim) << " VND\n";
    saveBudgets();
}

// Kiem tra trang thai ngan sach theo thang
void checkBudget() {
    int month = inputInt("  Nhap thang (1-12): ");
    if (month < 1 || month > 12) { cout << "  [!] Thang khong hop le (1-12)!\n"; return; }
    int year  = inputInt("  Nhap nam         : ");
    if (year < 1900 || year > 2100) { cout << "  [!] Nam phai tu 1900 den 2100!\n"; return; }

    cout << "\n  Trang thai ngan sach thang "
         << setfill('0') << setw(2) << month
         << "/" << setfill(' ') << year << ":\n";
    cout << "  " << left  << setw(17) << "Danh muc"
         << right << setw(19) << "Da chi (VND)"
         << setw(19) << "Han muc (VND)"
         << "  Trang thai\n";
    cout << "  " << string(74, '-') << "\n";

    bool found = false;
    for (int i = 0; i < gBudgets.size(); i++) {
        const Budget& b = gBudgets.at(i);
        if (b.month != month || b.year != year) continue;
        found = true;
        long long spent = getTotalSpent(b.category, month, year);
        string status;
        if      (spent > b.limit_val)          status = "[!!!] VUOT HAN MUC";
        else if (spent > b.limit_val * 8 / 10) status = "[~]  Sap toi han (>80%)";
        else                                   status = "[OK]";
        cout << "  " << left  << setw(17) << getCategoryName(b.category)
             << right << setw(19) << formatMoney(spent)
             << setw(19) << formatMoney(b.limit_val)
             << "  " << status << "\n";
    }
    if (!found)
        cout << "  Chua co ngan sach nao cho thang "
             << setfill('0') << setw(2) << month
             << "/" << setfill(' ') << year << ".\n";
}

// Bao cao cac danh muc vuot han muc
void reportExceededBudgets() {
    int month = inputInt("  Nhap thang (1-12): ");
    if (month < 1 || month > 12) { cout << "  [!] Thang khong hop le (1-12)!\n"; return; }
    int year  = inputInt("  Nhap nam         : ");
    if (year < 1900 || year > 2100) { cout << "  [!] Nam phai tu 1900 den 2100!\n"; return; }

    cout << "\n  Cac danh muc VUOT han muc thang "
         << setfill('0') << setw(2) << month
         << "/" << setfill(' ') << year << ":\n";
    cout << "  " << string(60, '-') << "\n";

    bool hasWarn = false;
    for (int i = 0; i < gBudgets.size(); i++) {
        const Budget& b = gBudgets.at(i);
        if (b.month != month || b.year != year) continue;
        long long spent = getTotalSpent(b.category, month, year);
        if (spent > b.limit_val) {
            cout << "  [!] " << left << setw(15) << getCategoryName(b.category)
                 << " Da chi: " << formatMoney(spent)
                 << " / Han muc: " << formatMoney(b.limit_val)
                 << "  (VUOT: " << formatMoney(spent - b.limit_val) << " VND)\n";
            hasWarn = true;
        }
    }
    if (!hasWarn) cout << "  Tat ca danh muc deu trong han muc!\n";
}

// Menu ngan sach
void budgetMenu() {
    int sub;
    do {
        cout << "\n+------ QUAN LY NGAN SACH ------+\n";
        cout << "  1. Xem han muc hien tai\n";
        cout << "  2. Dat / Sua han muc\n";
        cout << "  3. Kiem tra trang thai ngan sach\n";
        cout << "  4. Bao cao vuot han muc\n";
        cout << "  0. Quay lai menu chinh\n";
        sub = inputInt("  Chon (0-4): ");
        switch (sub) {
            case 1: listBudgets();             break;
            case 2: setBudget();               break;
            case 3: checkBudget();             break;
            case 4: reportExceededBudgets();   break;
            case 0:                            break;
            default: cout << "  Lua chon khong hop le.\n";
        }
    } while (sub != 0);
}


// ===================================================================
// MODULE 11: MAIN + MENU CHINH
// ===================================================================

void showMenu() {
    cout << "\n";
    cout << "  ============================================\n";
    cout << "      QUAN LY THU CHI CA NHAN  -  C++ Edition\n";
    cout << "  ============================================\n";
    cout << "   1. Them giao dich\n";
    cout << "   2. Xem danh sach giao dich\n";
    cout << "   3. Xem so du hien tai\n";
    cout << "   4. Thong ke theo ky\n";
    cout << "   5. Quan ly ngan sach (Budget)\n";
    cout << "   6. Bao cao vuot budget\n";
    cout << "   7. Bieu do chi tieu (Text)\n";
    cout << "   8. Tim kiem giao dich\n";
    cout << "   9. Xoa giao dich\n";
    cout << "  10. Quan ly danh muc\n";
    cout << "   0. Thoat chuong trinh\n";
    cout << "  ============================================\n";
    cout << "  Lua chon: ";
}

int main() {
    // Nap du lieu tu file vao bo nho
    loadCategories();
    loadTransactions();
    loadBudgets();

    cout << "  >> Da tai " << gCategories.size() << " danh muc, "
         << gTransactions.size() << " giao dich, "
         << gBudgets.size() << " ngan sach.\n";

    int choice = -1;
    while (choice != 0) {
        showMenu();
        if (!(cin >> choice)) {
            cin.clear();
            string tmp; getline(cin, tmp);
            choice = -1;
            cout << "  [!] Vui long nhap so!\n";
            continue;
        }
        clearInput();

        switch (choice) {
            case 1:  addTransaction();       break;
            case 2:  listTransactions();     break;
            case 3:  showBalance();          break;
            case 4:  statsMenu();            break;
            case 5:  budgetMenu();           break;
            case 6:
                cout << "\n+------ BAO CAO VUOT BUDGET ------+\n";
                reportExceededBudgets();
                break;
            case 7:
                cout << "\n+------ BIEU DO CHI TIEU ------+\n";
                printChart();
                break;
            case 8:  searchTransactions();   break;
            case 9:  deleteTransaction();    break;
            case 10: categoryMenu();         break;
            case 0:
                saveAll();
                cout << "\n  >> Da luu du lieu. Tam biet!\n";
                break;
            default:
                cout << "  [!] Lua chon khong hop le (0-10)!\n";
        }
    }
    return 0;
}
