#include <iostream>
#include <string>
using namespace std;

const int KAPASITAS_PARKIR = 5;
int slotTerpakai = 0;

// ===================== STRUKTUR DATA =====================

struct Kendaraan {
    string nopol;
    string pemilik;
    bool diParkiran; // true = Di Parkiran, false = Di Luar Parkiran
};

// Node BST
struct NodeBST {
    Kendaraan data;
    NodeBST* kiri;
    NodeBST* kanan;
};

// Node Stack
struct NodeStack {
    string nopol;
    string aksi; // "MASUK" atau "KELUAR"
    NodeStack* bawah;
};

// Node Queue
struct NodeQueue {
    string nopol;
    NodeQueue* next;
};

// ===================== BST =====================
// supaya pencarian nopol lebih cepat

NodeBST* buatNodeBST(Kendaraan k) {
    NodeBST* baru = new NodeBST();
    baru->data = k;
    baru->kiri = baru->kanan = nullptr;
    return baru;
}

NodeBST* insertBST(NodeBST* root, Kendaraan k) {
    if (!root) return buatNodeBST(k);
    if (k.nopol < root->data.nopol)
        root->kiri = insertBST(root->kiri, k);
    else if (k.nopol > root->data.nopol)
        root->kanan = insertBST(root->kanan, k);
    else
        cout << "Nomor polisi sudah terdaftar!\n";
    return root;
}

NodeBST* cariNodeBST(NodeBST* root, string nopol) {
    if (!root || root->data.nopol == nopol) return root;
    if (nopol < root->data.nopol) return cariNodeBST(root->kiri, nopol);
    return cariNodeBST(root->kanan, nopol);
}

// Cari node terkecil (untuk keperluan hapus)
NodeBST* nodeTerkecil(NodeBST* node) {
    while (node->kiri) node = node->kiri;
    return node;
}

NodeBST* hapusBST(NodeBST* root, string nopol, bool& berhasil) {
    if (!root) { berhasil = false; return nullptr; }

    if (nopol < root->data.nopol) {
        root->kiri = hapusBST(root->kiri, nopol, berhasil);
    } else if (nopol > root->data.nopol) {
        root->kanan = hapusBST(root->kanan, nopol, berhasil);
    } else {
        berhasil = true;
        if (!root->kiri) {
            NodeBST* temp = root->kanan;
            delete root;
            return temp;
        } else if (!root->kanan) {
            NodeBST* temp = root->kiri;
            delete root;
            return temp;
        }
        // Punya dua anak: ganti dengan suksesor in-order
        NodeBST* suksesor = nodeTerkecil(root->kanan);
        root->data = suksesor->data;
        bool dummy = true;
        root->kanan = hapusBST(root->kanan, suksesor->data.nopol, dummy);
    }
    return root;
}

void tampilInOrder(NodeBST* root) {
    if (!root) return;
    tampilInOrder(root->kiri);
    string status = root->data.diParkiran ? "Di Parkiran" : "Di Luar Parkiran";
    cout << "  " << root->data.nopol << " | " << root->data.pemilik << " | " << status << "\n";
    tampilInOrder(root->kanan);
}

bool bstKosong(NodeBST* root) { return root == nullptr; }


// STACK dipakai untuk menyimpan histori aksi
// jadi fitur undo bisa digunakan

struct Stack {
    NodeStack* top = nullptr;
};

void pushStack(Stack& s, string nopol, string aksi) {
    NodeStack* baru = new NodeStack();
    baru->nopol = nopol;
    baru->aksi = aksi;
    baru->bawah = s.top;
    s.top = baru;
}

bool popStack(Stack& s, string& nopol, string& aksi) {
    if (!s.top) return false;
    NodeStack* temp = s.top;
    nopol = temp->nopol;
    aksi = temp->aksi;
    s.top = temp->bawah;
    delete temp;
    return true;
}

bool stackKosong(Stack& s) { return s.top == nullptr; }

// QUEUE dipakai untuk kendaraan yg ngantri
// saat parkiran penuh

struct Queue {
    NodeQueue* depan = nullptr;
    NodeQueue* belakang = nullptr;
};

void enqueue(Queue& q, string nopol) {
    NodeQueue* baru = new NodeQueue();
    baru->nopol = nopol;
    baru->next = nullptr;
    if (!q.belakang) {
        q.depan = q.belakang = baru;
    } else {
        q.belakang->next = baru;
        q.belakang = baru;
    }
}

bool dequeue(Queue& q, string& nopol) {
    if (!q.depan) return false;
    NodeQueue* temp = q.depan;
    nopol = temp->nopol;
    q.depan = temp->next;
    if (!q.depan) q.belakang = nullptr;
    delete temp;
    return true;
}

bool queueKosong(Queue& q) { return q.depan == nullptr; }

// ===================== MENU FUNCTIONS =====================

void menuTambah(NodeBST*& root) {
    Kendaraan k;
    cout << "Nomor Polisi : ";
    cin.ignore();
    getline(cin, k.nopol);
    cout << "Nama Pemilik : ";
    getline(cin, k.pemilik);
    k.diParkiran = false;

    // Cek duplikat dulu
    if (cariNodeBST(root, k.nopol)) {
        cout << "Nomor polisi sudah terdaftar!\n";
        return;
    }
    root = insertBST(root, k);
    cout << "Data kendaraan berhasil ditambahkan.\n";
}

void menuTampil(NodeBST* root) {
    if (bstKosong(root)) {
        cout << "Belum ada data kendaraan.\n";
        return;
    }
    cout << "\n--- Daftar Kendaraan ---\n";
    cout << "  Nopol            | Pemilik            | Status\n";
    cout << "  -----------------|--------------------|------------------\n";
    tampilInOrder(root);
}

void menuMasuk(NodeBST* root, Stack& s, Queue& q) {
    string nopol;
    cout << "Nomor Polisi : ";
    cin.ignore();
    getline(cin, nopol);

    NodeBST* node = cariNodeBST(root, nopol);
    if (!node) {
        cout << "Kendaraan tidak ditemukan!\n";
        return;
    }
    if (node->data.diParkiran) {
        cout << "Kendaraan sudah berada di parkiran.\n";
        return;
    }

    if (slotTerpakai < KAPASITAS_PARKIR) {
        node->data.diParkiran = true;
        slotTerpakai++;
        pushStack(s, nopol, "MASUK");
        cout << "Kendaraan berhasil masuk.\n";
    } else {
        enqueue(q, nopol);
        cout << "Parkiran penuh, kendaraan masuk ke dalam antrian.\n";
    }
}

void menuKeluar(NodeBST* root, Stack& s, Queue& q) {
    string nopol;
    cout << "Nomor Polisi : ";
    cin.ignore();
    getline(cin, nopol);

    NodeBST* node = cariNodeBST(root, nopol);
    if (!node) {
        cout << "Kendaraan tidak ditemukan!\n";
        return;
    }
    if (!node->data.diParkiran) {
        cout << "Kendaraan tidak sedang di parkiran.\n";
        return;
    }

    node->data.diParkiran = false;
    slotTerpakai--;
    pushStack(s, nopol, "KELUAR");
    cout << "Kendaraan berhasil keluar.\n";

    // Proses antrian jika ada
    if (!queueKosong(q)) {
        string nopolAntrian;
        dequeue(q, nopolAntrian);
        NodeBST* nodeAntrian = cariNodeBST(root, nopolAntrian);
        if (nodeAntrian) {
            nodeAntrian->data.diParkiran = true;
            slotTerpakai++;
            pushStack(s, nopolAntrian, "MASUK");
            cout << "Kendaraan berikutnya langsung masuk ke area parkir dari antrian.\n";
        }
    }
}

void menuHapus(NodeBST*& root) {
    string nopol;
    cout << "Nomor Polisi : ";
    cin.ignore();
    getline(cin, nopol);

    NodeBST* node = cariNodeBST(root, nopol);
    if (!node) {
        cout << "Kendaraan tidak ditemukan!\n";
        return;
    }

    // Kurangi slot jika sedang parkir
    if (node->data.diParkiran) slotTerpakai--;

    bool berhasil = false;
    root = hapusBST(root, nopol, berhasil);
    if (berhasil) cout << "Data kendaraan berhasil dihapus.\n";
    else cout << "Kendaraan tidak ditemukan!\n";
}

void menuUndo(NodeBST* root, Stack& s) {
    if (stackKosong(s)) {
        cout << "Tidak ada aksi untuk di-undo.\n";
        return;
    }

    string nopol, aksi;
    popStack(s, nopol, aksi);

    NodeBST* node = cariNodeBST(root, nopol);
    if (!node) {
        cout << "Data kendaraan sudah tidak ada (mungkin sudah dihapus).\n";
        return;
    }

    if (aksi == "MASUK") {
        // Undo masuk = keluarkan
        node->data.diParkiran = false;
        slotTerpakai--;
        cout << "Undo: Kendaraan keluar dari parkiran.\n";
    } else {
        // Undo keluar = masukkan kembali
        node->data.diParkiran = true;
        slotTerpakai++;
        cout << "Undo: Kendaraan masuk kembali ke parkiran.\n";
    }
}

void menuAntrian(Queue& q) {
    if (queueKosong(q)) {
        cout << "Antrian kosong.\n";
        return;
    }
    cout << "\n--- Antrian Kendaraan ---\n";
    NodeQueue* cur = q.depan;
    int urut = 1;
    while (cur) {
        cout << "  " << urut++ << ". " << cur->nopol << " menunggu giliran masuk parkiran.\n";
        cur = cur->next;
    }
}

// ===================== MAIN =====================

int main() {
    NodeBST* root = nullptr;
    Stack histori;
    Queue antrian;

    int pilihan;
    cout << "======================================\n";
    cout << "  SISTEM PARKIR INAP - BANDARA GSA\n";
    cout << "  Kapasitas parkir: " << KAPASITAS_PARKIR << " kendaraan\n";
    cout << "======================================\n";

    do {
        cout << "\n--- MENU ---\n";
        cout << "1. Tambah Data Kendaraan\n";
        cout << "2. Tampil Semua Kendaraan\n";
        cout << "3. Kendaraan Masuk\n";
        cout << "4. Kendaraan Keluar\n";
        cout << "5. Hapus Data Kendaraan\n";
        cout << "6. Undo Aksi Terakhir\n";
        cout << "7. Tampil Antrian\n";
        cout << "0. Keluar\n";
        cout << "Slot terpakai: " << slotTerpakai << "/" << KAPASITAS_PARKIR << "\n";
        cout << "Pilihan: ";
        cin >> pilihan;

        switch (pilihan) {
            case 1: menuTambah(root); break;
            case 2: menuTampil(root); break;
            case 3: menuMasuk(root, histori, antrian); break;
            case 4: menuKeluar(root, histori, antrian); break;
            case 5: menuHapus(root); break;
            case 6: menuUndo(root, histori); break;
            case 7: menuAntrian(antrian); break;
            case 0: cout << "Program selesai.\n"; break;
            default: cout << "Pilihan tidak valid.\n";
        }
    } while (pilihan != 0);

    return 0;
}
