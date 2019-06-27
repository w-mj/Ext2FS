#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <stack>
#include "ext2/ext2_fs.h"
#include "dev/block_dev.h"
#include "dev/mock_disk.h"
#include "delog/delog.h"
#include "stat.h"
#include "delog/common.h"
#include "fs/real_file/real_file.h"
static int uid, gid;

struct OP {
    const char *name;
    VFS::DEntry* (*op)(VFS::DEntry *, const std::vector<std::string>&);
};
// 2 r 1 w 0 x
bool check(VFS::Inode *i, int pre) {
    if (i->uid == uid) {
        return i->mode & (0100 << pre);
    }
    if (i->gid == gid) {
        return i->mode & (0010 << pre);
    }
    return i->mode & (0001 << pre);
}

#define chk_r(i) check(i, 2)
#define chk_w(i) check(i, 1)
#define chk_x(i) check(i, 0)

#define pre(f, i) do {\
    if (!f(i)) {\
        std::cout << "Premission denine." << std::endl;\
        return cwd;\
    }\
} while(0)

static void ls_one(VFS::DEntry *x, bool l) {
    using namespace std;
    if (l) {
        x->inflate();
        switch (x->type) {
            case VFS::Directory:
                cout << "d";
                break;
            case VFS::RegularFile:
                cout << "-";
                break;
            case VFS::SymbolLink:
                cout << "l";
                break;
            default:
                cout << "?";
                break;
        }
        cout << previlege_to_str(x->inode->mode) << " " << x->inode->uid << " " << x->inode->gid;
        cout << " "  << x->inode->size << " " << x->inode->ctime << " " << x->name << endl;
    } else {
        cout << "(" << x->inode_n <<")"<< x->name << "  ";
    }
}

VFS::DEntry *ls(VFS::DEntry *cwd, const std::vector<std::string>& cmdd) {
    using namespace std;
    VFS::NameI *target = VFS::NameI::from_str(cmdd[cmdd.size() - 1]);
    auto t = cwd->get_child(target);
    // t = cwd;
    if (t == nullptr) {
        cout << "No such file or directory. " << cmdd[cmdd.size() - 1] << endl;
        return cwd;
    }
    t->inflate();
    if (!chk_r(t->inode)) {
        std::cout << "Premission denine." << std::endl;
        return cwd;
    }
    if (t->type == VFS::Directory) {
        t->load_children();
        for (auto& x: t->children) {
            ls_one(x, cmdd[1] == "-l");
        }
    } else  {
        ls_one(t, cmdd[1] == "-l");
    }
    cout << endl;
    delete target;
    return cwd;
}

VFS::DEntry *cd(VFS::DEntry *cwd,const std::vector<std::string>& cmdd) {
    using namespace std;
    if (cmdd[cmdd.size() - 1] == ".")
        return cwd;
    if (cmdd[cmdd.size() - 1] == "..")
        return cwd->parent;
    VFS::NameI *target = VFS::NameI::from_str(cmdd[cmdd.size() - 1]);
    auto ans = cwd->get_child(target);
    if (ans == nullptr) {
        cout << "No such directory " << cmdd[1] << endl;
        ans = cwd;
    } else if (ans->type != VFS::Directory) {
        cout << cmdd[1] << " is not a directory." << endl;
        ans = cwd;
    }
    delete target;
    return ans;
}

VFS::DEntry *mkdir(VFS::DEntry *cwd,const std::vector<std::string>& cmdd) {
    std::string name;
    VFS::NameI *target = VFS::NameI::from_str(cmdd[cmdd.size() - 1]);
    VFS::DEntry *d = cwd->get_path(target, &name);

    if (d == nullptr) {
        std::cout << "No such directory " << name << std::endl;
    }else 
    d->mkdir(name);
    delete target;
    return cwd;
}

VFS::DEntry *touch(VFS::DEntry *cwd,const std::vector<std::string>& cmdd) {
    std::string name;
    VFS::NameI *target = VFS::NameI::from_str(cmdd[cmdd.size() - 1]);
    VFS::DEntry *d = cwd->get_path(target, &name);

    if (d == nullptr) {
        std::cout << "No such directory " << name << std::endl;
    }else 
    d->create(name);
    delete target;
    return cwd;
}

VFS::DEntry *cat(VFS::DEntry *cwd,const std::vector<std::string>& cmdd) {
    VFS::NameI *target = VFS::NameI::from_str(cmdd[cmdd.size() - 1]);    
    auto ans = cwd->get_child(target);
    while (ans->type == VFS::SymbolLink)
        ans = ans->follow();
    
    VFS::File *fe = ans->open();
    if (fe == nullptr)
        std::cout << "No such file " << cmdd[1] << std::endl;
    else if (fe->type != VFS::RegularFile) {
        std::cout << cmdd[1] << " is not a regular file." << std::endl;
        delete fe;
    } else {
        _si(fe->size);
        char *buf = new char[fe->size];
        fe->read(buf, fe->size);
        fwrite(buf, fe->size, 1, stdout);
        putchar('\n');
        fflush(stdout);
        delete[] buf;
        delete fe;
    }
    delete target;
    return cwd;
}

VFS::DEntry *rm(VFS::DEntry *cwd,const std::vector<std::string>& cmdd) {
    VFS::DEntry *d;
    VFS::NameI *target = VFS::NameI::from_str(cmdd[cmdd.size() - 1]);
    VFS::DEntry *to_del = cwd->get_child(target, &d);
    if (to_del == nullptr) {
        std::cout << "No such file " << cmdd[cmdd.size() - 1] << std::endl;
    } else {
        if (to_del->type == VFS::Directory && cmdd[1] != "-r") {
            std::cout << cmdd[cmdd.size() - 1] << " is a directory." << std::endl;
        } else {
            if (to_del->type == VFS::Directory)
                to_del->unlink_children();
            d->unlink(to_del);
        }
    }
    delete target;
    return cwd;
}

VFS::DEntry *link(VFS::DEntry *cwd, const std::vector<std::string>& cmdd) {
    using namespace VFS;
    VFS::NameI *from = VFS::NameI::from_str(cmdd[cmdd.size() - 2]);

    DEntry *fe = cwd->get_child(from);
    if (fe == nullptr) {
        _log_info("%s is not exists.", cmdd[1].c_str());
        return cwd;
    }
    DEntry *te;
    std::string s;
    NameI *target = VFS::NameI::from_str(cmdd[cmdd.size() - 1]);
    if ((te = cwd->get_path(target, &s)) == nullptr) {
        _log_info("%s is already exists.", cmdd[2].c_str());
        return cwd;
    }
    fe->link(te, s);
    return cwd;
}

VFS::DEntry *mv(VFS::DEntry *cwd, const std::vector<std::string>& cmdd) {
    using namespace VFS;
    VFS::NameI *from = VFS::NameI::from_str(cmdd[cmdd.size() - 2]);

    DEntry *fe = cwd->get_child(from);
    if (fe == nullptr) {
        _log_info("%s is not exists.", cmdd[1].c_str());
        return cwd;
    }
    DEntry *te;
    std::string s;
    NameI *target = VFS::NameI::from_str(cmdd[cmdd.size() - 1]);
    if ((te = cwd->get_path(target, &s)) == nullptr) {
        _log_info("%s is not exists.", cmdd[2].c_str());
        return cwd;
    }
    fe->move(te, s);
    return cwd;
}

VFS::DEntry *cp(VFS::DEntry *cwd, const std::vector<std::string>& cmdd) {
    using namespace VFS;
    VFS::NameI *from = VFS::NameI::from_str(cmdd[cmdd.size() - 2]);

    DEntry *fe = cwd->get_child(from);
    if (fe == nullptr) {
        _log_info("%s is not exists.", cmdd[1].c_str());
        return cwd;
    }
    DEntry *te;
    std::string s;
    NameI *target = VFS::NameI::from_str(cmdd[cmdd.size() - 1]);
    if ((te = cwd->get_path(target, &s)) == nullptr) {
        _log_info("%s is not exists.", cmdd[2].c_str());
        return cwd;
    }
    fe->copy(te, s);
    return cwd;
}

VFS::DEntry *symlink(VFS::DEntry *cwd, const std::vector<std::string>& cmdd) {
    using namespace VFS;
    VFS::NameI *from = VFS::NameI::from_str(cmdd[cmdd.size() - 2]);

    DEntry *fe = cwd->get_child(from);
    if (fe == nullptr) {
        _log_info("%s is not exists.", cmdd[1].c_str());
        return cwd;
    }
    DEntry *te;
    std::string s;
    NameI *target = VFS::NameI::from_str(cmdd[cmdd.size() - 1]);
    if ((te = cwd->get_path(target, &s)) == nullptr) {
        _log_info("%s is not exists.", cmdd[2].c_str());
        return cwd;
    }
    te->symlink(fe, s);
    return cwd;
}

VFS::DEntry *mount(VFS::DEntry *cwd, const std::vector<std::string>& cmdd) {
    using namespace VFS;
    NameI *fs = VFS::NameI::from_str(cmdd[cmdd.size() - 2]);
    NameI *dir = VFS::NameI::from_str(cmdd[cmdd.size() - 1]);
    DEntry *fs_e = cwd->get_child(fs);
    DEntry *mnt_e = cwd->get_child(dir);
    if (fs_e == nullptr) {
        std::cout << "no such device " << cmdd[cmdd.size() - 2] << std::endl;
        return cwd;
    }
    if (mnt_e == nullptr) {
        std::cout << "no such diectory " << cmdd[cmdd.size() - 1] << std::endl;
        return cwd;
    }
    File *f = fs_e->open();
    Dev::BlockDevice *d = new Dev::MockDisk(*f);
    FS* new_fs = new EXT2::EXT2_FS(d);
    new_fs->mount();
    new_fs->root->load_children();
    new_fs->root->parent = mnt_e->parent;
    // mnt_e->inode = new_fs->root->inode;
    // mnt_e->children = new_fs->root->children;
    new_fs->root->name = mnt_e->name;
    mnt_e->parent->children.push_back(new_fs->root);
    mnt_e->parent->children.remove(mnt_e);
    return cwd;
}

VFS::DEntry *umount(VFS::DEntry *cwd, const std::vector<std::string>& cmdd) {
    using namespace VFS;
    NameI *dir = VFS::NameI::from_str(cmdd[cmdd.size() - 1]);
    DEntry *mnt_e = cwd->get_child(dir);
    if (mnt_e == nullptr) {
        std::cout << "no such diectory " << cmdd[cmdd.size() - 1] << std::endl;
        return cwd;
    }
    DEntry *t = mnt_e->parent;
    t->children.clear();
    t->load_children();
    mnt_e->fs->dev->close();
    delete mnt_e->fs;
    return cwd;
}

VFS::DEntry *echo(VFS::DEntry *cwd, const std::vector<std::string>& cmdd) {
    if (cmdd.size() != 4)
        return cwd;
    const std::string& text = cmdd[1];
    VFS::NameI *fi = VFS::NameI::from_str(cmdd[cmdd.size() - 1]);
    VFS::DEntry *file = cwd->get_child(fi);
    if (file == nullptr) {
        std::cout << "no such file " << cmdd[cmdd.size() - 1] << std::endl;
        return cwd;
    }
    if (file->type != VFS::RegularFile) {
        std::cout << cmdd[cmdd.size() - 1] << " is not a file." << std::endl;
        return cwd;
    }
    VFS::File *f = file->open();
    if (cmdd[2] == ">>") {
        f->seek(0, SEEK_END);
    } else {
        f->seek(0, SEEK_SET);
    }
    f->write(text.c_str(), text.size());
    f->close();
    return cwd;
}

VFS::DEntry *chmod(VFS::DEntry *cwd, const std::vector<std::string>& cmdd) {
    VFS::NameI *fi = VFS::NameI::from_str(cmdd[cmdd.size() - 1]);
    VFS::DEntry *file = cwd->get_child(fi);
    return cwd;
}

VFS::DEntry *chown(VFS::DEntry *cwd, const std::vector<std::string>& cmdd) {
    if (cmdd.size() != 4)
        return cwd;
    VFS::NameI *fi = VFS::NameI::from_str(cmdd[cmdd.size() - 1]);
    VFS::DEntry *file = cwd->get_child(fi);
    file->inflate();
    pre(chk_w, file->inode);
    file->inode->uid = stoi(cmdd[1]);
    file->inode->gid = stoi(cmdd[2]);
    file->inode->dirty();
    return cwd;
}

OP ops[] = {
    {"ls", ls},
    {"cd", cd},
    {"mkdir", mkdir},
    {"touch", touch},
    {"cat", cat},
    {"rm", rm},
    {"link", link},
    {"mv", mv},
    {"cp", cp},
    {"ln", symlink},
    {"mount", mount},
    {"umount", umount},
    {"echo", echo},
    {"chown", chown},
    {"unknown", nullptr}
};

struct User {
    std::string uname;
    int uid, gid;
    std::string passwd;
    User(std::vector<std::string> s) {
        uname = s[0];
        uid = stoi(s[1]);
        gid = stoi(s[2]);
        passwd = s[3];
        // std::cout << uname << " " << uid << " " << gid << " " << passwd << std::endl;
    }
};

void login(VFS::DEntry *root) {
    auto de = root->get_child("passwd");
    auto pass = de->open();
    char buf[pass->size];
    pass->read(buf, pass->size);
    std::string s(buf, pass->size);
    auto sp = split(s, '\n');
    std::vector<User> uv;
    for (auto x: sp) {
        uv.emplace_back(split(x, ':'));
    }
    std::string buf2, buf1;
    bool success = false;
    do {
        std::cout << "user: ";
        std::cin >> buf2;
        std::cout << "password: ";
        std::cin >> buf1;
        for (auto x: uv) {
            if (x.uname == buf2 && x.passwd == buf1) {
                uid = x.uid;
                gid = x.gid;
                success = true;
            }
        }
    } while (!success);
}

#include <unistd.h>
int main(void) {
    using namespace std;

    RealFile::RealFile real_disk("fs/ext2/fs");
    Dev::BlockDevice *dev = new Dev::MockDisk(real_disk);

    EXT2::EXT2_FS *ext2_fs = new EXT2::EXT2_FS(dev);
    VFS::FS *fs = ext2_fs;
    cout << "System up." << endl;
    cout << "File System at " << "fs/ext2/fs" << endl;
    string cmd;
    VFS::DEntry *cwd = nullptr;

    fs->mount();
    cwd = fs->root;
    login(cwd);


    cout << "/$ ";
    while (getline(cin, cmd)) {
        if (cmd.size() == 0)
            continue;
        vector<string> cmdd = split(cmd, ' ');
        if (cmd == "mount") {
            fs->mount();
            cwd = fs->root;
            cout << "mount disk at /" << endl;
        } else if (fs->status == VFS::FS_unmounted) {
            cout << "Please mount fs first." << endl;
        } else if (cmdd[0] == "exit") {
            break; 
        } else if (cmdd[0] == "dump") {
            MM::Buf buf(1024);
            if (cmdd[1] == "i")
                ext2_fs->dev->read(buf, ext2_fs->inode_to_pos(stoi(cmdd[2])), 1024);
            else 
                ext2_fs->dev->read(buf, ext2_fs->block_to_pos(stoi(cmdd[2])), 1024);
            _sa(buf.data, 1024);
        } else {
            int i = 0;
            while (ops[i].op != nullptr && strcmp(ops[i].name, cmdd[0].c_str()) != 0) {
                i++;
            }
            if (ops[i].op == nullptr)
                cout << "Unknown command " << cmdd[0] << endl;
            else {
                VFS::NameI *path = nullptr;
                if (cmdd.size() > 1) {
                    path = VFS::NameI::from_str(cmdd[cmdd.size() - 1]);
                } else {
                    cmdd.push_back(".");
                    path = VFS::NameI::from_str(".");
                }
                if (path == nullptr) {
                    cout << "Wrong path " << cmdd[cmdd.size() - 1] << endl;
                } else {
                    cwd = ops[i].op(cwd, cmdd);
                }
                delete path;
            }
        }
        
        if (cwd != nullptr) {
            cout << cwd->printed_path() << "$ ";
        } else
            cout << "[umount]$ ";
    }
    dev->close();
}