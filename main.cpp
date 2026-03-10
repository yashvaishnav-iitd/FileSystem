#include <iostream>
#include <string>
#include <vector>
#include <ctime>
#include <sstream>
#include <chrono>

using namespace std;

// Forward declaration
class File;

// Helper function to get current time in milliseconds since epoch
long long getCurrentTimeMillis() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

// This represents a single version of a file
struct TreeNode {
    int version_id;
    string content;
    string message;
    TreeNode* parent; // FIXED: Reordered to match initialization order
    vector <TreeNode*> children;
    long long created_timestamp;
    long long snapshot_timestamp;

    TreeNode(int id, string cont, TreeNode* p = nullptr)
        : version_id(id), content(cont), parent(p),
        created_timestamp(getCurrentTimeMillis()), snapshot_timestamp(0) {}

}; 

// Custom Hashmap implementation
// I am using chaining with a simple Hash Function 
template <typename K, typename V>
class HashMap {
private:
    struct HashNode {
        K key;
        V value;
        HashNode* next;
        HashNode(K k, V v)
            : key(k), value(v), next(nullptr) {}
    };

    vector <HashNode*> table;
    int capacity;
    int size;

    //Hash function for integer keys
    int HashCode(int key) {
        return key % capacity;

    }
    // Hash function for string keys (djb2 hash function)
    int HashCode(const string& key) {
        unsigned long hash = 5381;
        for (char c : key) {
            hash = (hash << 5) + hash + c;  // (hash<<5) = hash*32 and 'c' is not added as a character instead its ASCII value is added.
        }
        return hash % capacity;
    }


public:
    HashMap(int cap = 100) : capacity(cap), size(0) {
        table.resize(capacity, nullptr);
    }

    ~HashMap() {
        for (int i = 0; i < capacity; i++) {
            HashNode* entry = table[i];
            while (entry != nullptr) {
                HashNode* prev = entry;
                entry = entry->next;
                delete prev;

            }
        }
    }
    void insert(K key, V value) {
        int HashIndex = HashCode(key);
        HashNode* prev = nullptr;
        HashNode* entry = table[HashIndex];

        // The hash%capcity we are returning as HashIndex may be same for 2 different keys so we are creating a linked list to store the values 
        while (entry != nullptr && entry->key != key) {
            prev = entry;
            entry = entry->next;

        }
        if (entry == nullptr) {
            entry = new HashNode(key, value);
            if (prev == nullptr) {
                table[HashIndex] = entry;
            }
            else {
                prev->next = entry;
            }
            size++;
        }
        else {
            //key already exists so just update the value
            entry->value = value;
        }
    }
    V get(K key) {
        int HashIndex = HashCode(key);
        HashNode* entry = table[HashIndex];

        while (entry != nullptr) {
            if (entry->key == key) {
                return entry->value;

            }
            entry = entry->next;
        }
        return V(); //Return default value if not found
    }
};

// Custom Max-Heap Implementation
// This root of this gives maximum value present
template <typename T, typename Comparator>
class MaxHeap {
private:
    vector <T> heap;
    Comparator comparator;

    void HeapifyUP(size_t index) { 
        if (index != 0 && comparator(heap[index], heap[(index - 1) / 2])) {
            swap(heap[index], heap[(index - 1) / 2]);
            HeapifyUP((index - 1) / 2);
        }
    }
    void HeapifyDown(size_t index) { 
        size_t left = 2 * index + 1;
        size_t right = 2 * index + 2;
        size_t largest = index;

        if (left < heap.size() && comparator(heap[left], heap[largest])) {
            largest = left;
        }
        if (right < heap.size() && comparator(heap[right], heap[largest])) {
            largest = right;
        }
        if (largest != index) {
            swap(heap[index], heap[largest]);
            HeapifyDown(largest);
        }
    }
public:
    MaxHeap(Comparator comp = Comparator()) : comparator(comp) {}

    MaxHeap(const MaxHeap& other)
        : heap(other.heap), comparator(other.comparator) {} 

    void insert(T element) {
        heap.push_back(element);
        HeapifyUP(heap.size() - 1);
    }

    bool isEmpty() const {    //Here we wrote 'const' because our we are passing the heap as const and don't want to change it
        return heap.empty();
    }

    T extractMax() {
        if (isEmpty()) { 
            cout << "Error: Attempted to extract from an empty heap." << endl;
            return T(); // Return default value instead of throwing error
        }
        T max_element = heap[0];
        heap[0] = heap.back();
        heap.pop_back();
        HeapifyDown(0);
        return max_element;
    }

    vector <T> getTopN(int n) {
        MaxHeap<T, Comparator> temp_heap = *this;
        vector <T> top_n;
        int count = 0;
        while (count < n && !temp_heap.isEmpty()) { 
            top_n.push_back(temp_heap.extractMax());
            count++;

        }
        return top_n;
    }

    void clear() {
        heap.clear();
    }

};

//File and FilSystem Classes

//File manages the version history of a single file
class File {
public:
    string filename; 
    TreeNode* root;
    TreeNode* active_version;
    HashMap <int, TreeNode*> version_map;
    int total_versions;
    long long last_modification_time;


    File(const string& name)
        : filename(name), total_versions(1) { 
        root = new TreeNode(0, "");
        root->message = "Initial commit";
        root->snapshot_timestamp = getCurrentTimeMillis();
        active_version = root;
        version_map.insert(0, root);
        last_modification_time = getCurrentTimeMillis();

    }

    ~File() {
        deleteTree(root); 
    }
private:
    void deleteTree(TreeNode* node) { 
        if (!node) return;
        for (TreeNode* child : node->children) {
            deleteTree(child);

        }
        delete node;
    }
};


struct RecentFileComparator {
    bool operator () (File* a, File* b) const {
        return a->last_modification_time > b->last_modification_time;
    }
};
struct BiggestTreesComparator {
    bool operator() (File* a, File* b) const {
        return a->total_versions > b->total_versions;
    }
};


string formatTimestamp(long long timestamp_ms) {
    if (timestamp_ms == 0) return "N/A";

    // 1. Get the seconds and milliseconds parts
    time_t seconds_part = timestamp_ms / 1000;
    int millis_part = timestamp_ms % 1000;

    // 2. Format the seconds part to a string using the local timezone
    char time_buf[80];
    strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", localtime(&seconds_part));

    // 3. Combine the parts into the final string
    stringstream ss;
    ss << time_buf << ".";

    // Add leading zeros to milliseconds manually to ensure it's 3 digits
    if (millis_part < 10) {
        ss << "00";
    }
    else if (millis_part < 100) {
        ss << "0";
    }
    ss << millis_part;

    return ss.str();
}

class FileSystem {
private:
    HashMap<string, File*> files;


    MaxHeap <File*, RecentFileComparator> recent_files_heap; 
    MaxHeap <File*, BiggestTreesComparator> biggest_trees_heap; 

    vector <File*> all_files_list;

    void rebuildHeaps() { 
        recent_files_heap.clear();
        biggest_trees_heap.clear();

        for (File* f : all_files_list) {
            recent_files_heap.insert(f);
            biggest_trees_heap.insert(f);
        }
    }

    void updateAnalytics(File* file) { 
        file->last_modification_time = getCurrentTimeMillis();
        rebuildHeaps();
    }

public:
    FileSystem()
        : files(20) {}

    ~FileSystem() {
        for (File* f : all_files_list) {
            delete f;
        }
    }

    void createFile(const string& filename) { 
        if (files.get(filename) != nullptr) {
            cout << "Error: File '" << filename << "' already exists. " << endl;
            return;
        }
        File* new_file = new File(filename);
        files.insert(filename, new_file);
        all_files_list.push_back(new_file);
        updateAnalytics(new_file);
        cout << "File '" << filename << "' created. " << endl;
    }
    void readFile(const string& filename) { 
        File* file = files.get(filename);
        if (!file) {
            cout << "Error: File " << filename << " not found. " << endl;
            return;

        }
        if (file->active_version->content==""){
            cout << filename << " is Empty.";
        }
        cout << file->active_version->content << " " << endl;
    }

    void modifyFile(const string& filename, const string& content, bool append) { 
        File* file = files.get(filename);
        if (!file) {
            cout << "Error: File '" << filename << "' not found. " << endl;
            return;
        }

        if (file->active_version->snapshot_timestamp != 0) {
            int new_version_id = file->total_versions;
            TreeNode* new_version = new TreeNode(new_version_id, content, file->active_version);
            if (append) {
                new_version->content = file->active_version->content + content;
            }
            file->active_version->children.push_back(new_version); 
            file->active_version = new_version;
            file->version_map.insert(new_version_id, new_version);
            file->total_versions++;
            cout << "Created new version " << new_version_id << " for file '" << filename << "'. " << endl;
        }
        else {
            if (append) {
                file->active_version->content += content;
            }
            else {
                file->active_version->content = content;
            }

            cout << "Modified active version of file '" << filename << "'. " << endl;
        }
        updateAnalytics(file);
    }

    void snapshotFile(const string& filename, const string& message) { 
        File* file = files.get(filename);

        if (!file) {
            cout << "Error: File '" << filename << "' not found. " << endl;
            return;
        }
        if (file->active_version->snapshot_timestamp != 0) {
            cout << "Error: Version " << file->active_version->version_id << " is already a snapshot. " << endl;
            return;
        }
        file->active_version->snapshot_timestamp = getCurrentTimeMillis();
        file->active_version->message = message;
        cout << "Snapshot created for file '" << filename << "' with version " << file->active_version->version_id << ". " << endl;
        updateAnalytics(file);
    }
    void rollbackFile(const string& filename, int versionID = -1) { 
        File* file = files.get(filename);
        if (!file) {
            cout << "Error: File '" << filename << "' not found. " << endl;
            return;
        }
        TreeNode* target_version = nullptr;

        if (versionID == -1) {
            if (file->active_version->parent) {
                target_version = file->active_version->parent;

            }
            else {
                cout << "Error: Cannot rollback from root version. " << endl;
                return;
            }
        }
        else {
            target_version = file->version_map.get(versionID);
            if (!target_version) {
                cout << "Error: Version ID " << versionID << " not found for file '" << filename << "'. " << endl;
                return;
            }
        }
        file->active_version = target_version;
        cout << "File '" << filename << "' rolled back to version " << file->active_version->version_id << ". " << endl;
        updateAnalytics(file);
    }

    void showHistory(const string& filename) { 
        File* file = files.get(filename);
        if (!file) {
            cout << "Error: File '" << filename << "' not found. " << endl;
            return;
        }
        cout << "History for '" << filename << "' (from active to root): " << endl;
        TreeNode* current = file->active_version; 
        while (current != nullptr) { 
            if (current->snapshot_timestamp != 0) {
                cout << "  - Version: " << current->version_id
                    << ", Timestamp: " << formatTimestamp(current->snapshot_timestamp)
                    << ", Message: " << current->message << endl; 
            }
            current = current->parent; 
        }
    }
    void showRecentFiles(int num) { 
        cout << "Top " << num << " most recently modified files: " << endl;
        vector <File*> top_files = recent_files_heap.getTopN(num);
        for (File* f : top_files) {
            cout << "  - " << f->filename << " (Last modified: " << formatTimestamp(f->last_modification_time) << ") " << endl; 
        }
    }
    void showBiggestTrees(int num) {
        cout << "Top " << num << " files with the most versions: " << endl;
        vector<File*> top_files = biggest_trees_heap.getTopN(num);
        for (File* f : top_files) {
            cout << "  - " << f->filename << " (" << f->total_versions << " versions) " << endl; 
        }
    }
};

//Main function 
void printHelp() {
    cout << "\n--- Time-Travelling File System ---" << endl;
    cout << "Available Commands:" << endl;
    cout << "  CREATE <filename>" << endl;
    cout << "  READ <filename>" << endl;
    cout << "  INSERT <filename> <content>" << endl;
    cout << "  UPDATE <filename> <content>" << endl;
    cout << "  SNAPSHOT <filename> <message>" << endl;
    cout << "  ROLLBACK <filename> [versionID]" << endl;
    cout << "  HISTORY <filename>" << endl;
    cout << "  RECENT_FILES [num]" << endl;
    cout << "  BIGGEST_TREES [num]" << endl;
    cout << "  HELP" << endl;
    cout << "  EXIT" << endl;
    cout << "-----------------------------------" << endl;
}

int main() {
    FileSystem fs;
    string line;

    printHelp();

    while (true) {
        cout << "> ";
        if (!getline(cin, line)) {
            break;
        }

        stringstream ss(line);
        string command;
        ss >> command;

        if (command == "EXIT") {
            cout << "EXITING "<<endl;
            break;
        }
        else if (command == "HELP") {
            printHelp();
        }
        else if (command == "CREATE") {
            string filename;
            ss >> filename;
            if (filename.empty()) cout << "Usage: CREATE <filename>" << endl;
            else fs.createFile(filename); 
        }
        else if (command == "READ") {
            string filename;
            ss >> filename;
            if (filename.empty()) cout << "Usage: READ <filename>" << endl;
            else fs.readFile(filename); 
        }
        else if (command == "INSERT" || command == "UPDATE") {
            string filename, content;
            ss >> filename;
            getline(ss >> ws, content);
            if (filename.empty() || content.empty()) cout << "Usage: " << command << " <filename> <content>" << endl;
            else fs.modifyFile(filename, content, command == "INSERT");
        }
        else if (command == "SNAPSHOT") {
            string filename, message;
            ss >> filename;
            getline(ss >> ws, message);
            if (filename.empty() || message.empty()) cout << "Usage: SNAPSHOT <filename> <message>" << endl;
            else fs.snapshotFile(filename, message); 
        }
        else if (command == "ROLLBACK") {
            string filename;
            int versionID = -1;
            ss >> filename;
            if (filename.empty()) {
                cout << "Usage: ROLLBACK <filename> [versionID]" << endl;
            }
            else {
                if (!(ss >> versionID)) {
                    versionID = -1;
                }
                fs.rollbackFile(filename, versionID); 
            }
        }
        else if (command == "HISTORY") {
            string filename;
            ss >> filename;
            if (filename.empty()) cout << "Usage: HISTORY <filename>" << endl;
            else fs.showHistory(filename); 
        }
        else if (command == "RECENT_FILES") {
            int num = 5; // default
            ss >> num;
            fs.showRecentFiles(num); 
        }
        else if (command == "BIGGEST_TREES") {
            int num = 5; // default
            ss >> num;
            fs.showBiggestTrees(num);
        }
        else {
            if (!command.empty()) {
                cout << "Unknown command: " << command << ". Type HELP for a list of commands." << endl;
            }
        }
    }

    return 0;
}

