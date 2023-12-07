#include <iostream>
#include <memory>
#include <vector>

enum Color {
    RED = 0,
    BLACK,
};

struct Node {
    int value;
    Color color;
    std::shared_ptr<Node> parent;
    std::shared_ptr<Node> left;
    std::shared_ptr<Node> right;

    Node() : value(0), color(RED) {}
    Node(int _value) : value(_value), color(RED) {}
};

class RBTree {
public:
    RBTree();
    ~RBTree();

    void Insert(int value);
    bool Delete(int value);

    friend std::ostream& operator<<(std::ostream& o, const RBTree& tree);

private:
    void InsertRecurse(std::shared_ptr<Node>& src_node,
                       std::shared_ptr<Node>& insert_node);

    void AdjustTree(std::shared_ptr<Node>& node);

    std::shared_ptr<Node> DeleteRecurse(std::shared_ptr<Node>& node, int value);

    void DeleteFix(std::shared_ptr<Node>& node);

    void LeftRotate(std::shared_ptr<Node>& node);
    void RightRotate(std::shared_ptr<Node>& node);

    std::shared_ptr<Node> FixNode(std::shared_ptr<Node>& del_node);
    std::shared_ptr<Node> Successor(std::shared_ptr<Node>& node);

private:
    std::shared_ptr<Node> root_;
};

RBTree::RBTree() {}

RBTree::~RBTree() {}

void RBTree::Insert(int value) {
    // 1. 插入结点为根结点
    if (!root_) {
        root_        = std::make_shared<Node>(value);
        root_->color = BLACK;

        return;
    }

    auto insert_node = std::make_shared<Node>(value);
    InsertRecurse(root_, insert_node);
    // 插入结点之后需要进行调整，插入的结点均为红色，没有增加黑长，
    // 只可能产生冲突，就是插入结点的颜色和父结点的颜色均为红色
    AdjustTree(insert_node);
    root_->color = BLACK;
}

bool RBTree::Delete(int value) {
    if (!root_) {
        return false;
    }

    auto del_node = DeleteRecurse(root_, value);
    if (!del_node) {
        return false;
    }

    // 获取删除需要修复的结点
    auto fix_node = FixNode(del_node);
    // 如果需要修复的结点与需要删除的结点不一致，那么迁移修复结点的信息到删除结点
    if (fix_node != del_node) {
        del_node->value = fix_node->value;
    }

    DeleteFix(fix_node);
    if (fix_node == root_) {
        root_.reset();
    } else {
        auto parent = fix_node->parent;
        if (parent->left == fix_node) {
            parent->left.reset();
        } else {
            parent->right.reset();
        }
    }

    return true;
}

std::ostream& operator<<(std::ostream& o, const RBTree& tree) {
    if (!tree.root_) {
        return o;
    }

    // 采用层序遍历的方式显示tree
    std::vector<std::vector<std::shared_ptr<Node>>> level_nodes_list;
    std::vector<std::shared_ptr<Node>> first_level = {tree.root_};
    level_nodes_list.emplace_back(first_level);

    std::vector<std::shared_ptr<Node>> last_level_nodes = first_level;
    do {
        std::vector<std::shared_ptr<Node>> cur_level_nodes;
        for (auto& last_level_node : last_level_nodes) {
            if (last_level_node->left) {
                cur_level_nodes.emplace_back(last_level_node->left);
            }

            if (last_level_node->right) {
                cur_level_nodes.emplace_back(last_level_node->right);
            }
        }

        if (cur_level_nodes.empty()) {
            break;
        }

        level_nodes_list.emplace_back(cur_level_nodes);
        last_level_nodes = cur_level_nodes;
    } while (true);

    for (const auto& level_nodes : level_nodes_list) {
        for (const auto& level_node : level_nodes) {
            o << level_node->value << ":" << level_node->color << "\t";
        }
        o << std::endl;
    }

    return o;
}

void RBTree::InsertRecurse(std::shared_ptr<Node>& src_node,
                           std::shared_ptr<Node>& insert_node) {
    if (!src_node) {
        src_node = insert_node;
        return;
    }

    if (src_node->value == insert_node->value) {
        std::cerr << "value has been inserted!\n";
        return;
    }

    if (src_node->value > insert_node->value) {
        InsertRecurse(src_node->left, insert_node);
    } else {
        InsertRecurse(src_node->right, insert_node);
    }

    if (!insert_node->parent) {
        insert_node->parent = src_node;
    }
}

void RBTree::AdjustTree(std::shared_ptr<Node>& node) {
    auto parent = node->parent;
    if (!parent || BLACK == parent->color) {
        return;
    }

    auto grandparent = parent->parent;
    std::shared_ptr<Node> uncle;
    do {
        if (parent == grandparent->left) {
            if (grandparent->right && RED == grandparent->right->color) {
                uncle = grandparent->right;
                break;
            }

            if (node == parent->left) {
                // 1. 左左
                parent->color      = BLACK;
                grandparent->color = RED;
                RightRotate(grandparent);
                if (root_ == grandparent) {
                    root_ = parent;
                }
            } else {
                // 2. 左右
                LeftRotate(parent);  // ==> 左左，对变成左左以后的parent进行调整
                AdjustTree(parent);
            }
        } else {
            if (grandparent->left && RED == grandparent->left->color) {
                uncle = grandparent->left;
                break;
            }

            if (node == parent->right) {
                // 3. 右右
                parent->color      = BLACK;
                grandparent->color = RED;
                LeftRotate(grandparent);
                if (root_ == grandparent) {
                    root_ = parent;
                }
            } else {
                // 4. 右左
                RightRotate(parent);
                AdjustTree(parent);
            }
        }
    } while (false);

    if (uncle) {
        parent->color = BLACK;
        uncle->color  = BLACK;

        grandparent->color = RED;
        AdjustTree(grandparent);
    }
}

std::shared_ptr<Node> RBTree::DeleteRecurse(std::shared_ptr<Node>& node,
                                            int value) {
    if (!node) {
        return nullptr;
    }

    if (node->value == value) {
        return node;
    }

    if (value < node->value) {
        return DeleteRecurse(node->left, value);
    } else {
        return DeleteRecurse(node->right, value);
    }
}

void RBTree::DeleteFix(std::shared_ptr<Node>& node) {
    // 删除一个结点的时候，需要递归的去进行结点的修复
    if (root_ == node || RED == node->color) {
        // 如果是根结点，表示所有的子树都已经修复完成
        // 在修复的过程中如果发现修复的颜色为红色，那么是在递归的过程中
        // 发现的一个红色结点，将该结点的颜色赋值成黑色，那么也可以弥补
        // 当前子树黑长-1的问题
        node->color = BLACK;

        return;
    }

    // 不是根结点、并且结点颜色是黑色，那么进入到删除修复的步骤
    auto parent = node->parent;
    if (parent->left == node) {
        auto brother = parent->right;
        // 1.
        // 兄弟结点的颜色为红色，通过旋转操作将其转换成兄弟结点的颜色为黑色在进行操作
        // 该操作不改变左右子树的黑长
        if (RED == brother->color) {
            brother->color = BLACK;
            parent->color  = RED;
            LeftRotate(parent);

            brother = parent->right;
        }

        // 如果此时兄弟结点的两个子结点均为黑色，或者均为空，那么只要将兄弟子树的长度-1，也就是
        // 兄弟结点变成红色，便可以实现左右子树的长度保持一致，均-1
        if ((!brother->left && !brother->right) ||
            (brother->left && BLACK == brother->left->color && brother->right &&
             BLACK == brother->right->color)) {
            brother->color = RED;

            // parent的左右子树长度保持一致了，但是parent这颗子树的长度-1，所以需要在递归修复parent
            // 这颗子树
            DeleteFix(parent);
        } else {
            // 如果兄弟子树中有一个结点不为黑色，那么采取另一种使两颗子树黑长保持一致的方式，
            // 便是让该删除子树的黑长+1，那么整体也可以保持黑长一致
            if (!brother->right || RED != brother->right->color) {
                // 也就是该子树的右子树不满足要求，因为需要进行右旋使删除子树的黑长+1
                auto bro_left = brother->left;
                if (!bro_left) {
                    // 不可能有这种情况
                    std::cerr << "illegal tree!\n";
                    return;
                }

                bro_left->color = BLACK;
                brother->color  = RED;
                RightRotate(brother);
            }

            // 到达该处的tree的结构满足brother的右结点是红色结点
            auto bro_right = brother->right;
            if (!bro_right) {
                // 不可能有这种情况
                std::cerr << "illegal tree!\n";
                return;
            }

            // 使删除结点的数的黑长+1
            brother->color   = parent->color;
            parent->color    = BLACK;
            bro_right->color = BLACK;
            LeftRotate(brother);
        }
    } else {
        // 与前者成镜像对称关系
        auto brother = parent->left;
        if (RED == brother->color) {
            brother->color = BLACK;
            parent->color  = RED;
            RightRotate(parent);

            brother = parent->left;
        }

        if ((!brother->left && !brother->right) ||
            (brother->left && BLACK == brother->left->color && brother->right &&
             BLACK == brother->right->color)) {
            brother->color = RED;

            DeleteFix(parent);
        } else {
            if (!brother->left || RED != brother->left->color) {
                auto bro_right = brother->right;
                if (!bro_right) {
                    std::cerr << "illegal tree!\n";
                    return;
                }

                bro_right->color = BLACK;
                brother->color   = RED;
                LeftRotate(brother);
            }

            auto bro_left = brother->left;
            if (!bro_left) {
                std::cerr << "illegal tree!\n";
                return;
            }

            brother->color  = parent->color;
            parent->color   = BLACK;
            bro_left->color = BLACK;
            RightRotate(brother);
        }
    }
}

void RBTree::LeftRotate(std::shared_ptr<Node>& node) {
    auto right = node->right;
    if (!right) {
        std::cerr << "illegal left rotate, has no right child!\n";
        return;
    }

    auto parent = node->parent;
    if (parent && node == parent->left) {
        parent->left = right;
    } else if (parent && node == parent->right) {
        parent->right = right;
    }

    right->parent = parent;
    node->parent  = right;
    node->right   = right->left;
    right->left   = node;
}

void RBTree::RightRotate(std::shared_ptr<Node>& node) {
    auto left = node->left;
    if (!left) {
        std::cerr << "illegal right rotate, has no left child!\n";
        return;
    }

    auto parent = node->parent;
    if (parent && node == parent->left) {
        parent->left = left;
    } else if (parent && node == parent->right) {
        parent->right = left;
    }

    left->parent = node->parent;
    node->parent = left;
    node->left   = left->right;
    left->right  = node;
}

std::shared_ptr<Node> RBTree::FixNode(std::shared_ptr<Node>& del_node) {
    // 1. 待删除结点是叶子结点
    if (!del_node->left && !del_node->right) {
        return del_node;
    }

    // 2. 待删除结点有两个叶子结点
    if (del_node->left && del_node->right) {
        // 找后继结点
        auto successor = Successor(del_node);
        return FixNode(successor);
    }

    // 3. 待删除结点只有左子结点
    if (del_node->left && !del_node->right) {
        return del_node->left;
    }

    // 4. 待删除结点只有右子结点
    if (del_node->right && !del_node->left) {
        return del_node->right;
    }

    // 上述四种情况是全部的情况，不可能进入到以下情况
    return nullptr;
}

std::shared_ptr<Node> RBTree::Successor(std::shared_ptr<Node>& node) {
    auto successor = node->right;
    if (!successor) {
        return nullptr;
    }

    while (successor->left) {
        successor = successor->left;
    }

    return successor;
}

int main(int argc, char* argv[]) {
    RBTree tree;
    std::vector<int> insert_values = {10, 5, 20, 1, 2, 7, 15, 30};
    for (auto value : insert_values) {
        tree.Insert(value);
    }

    std::cout << tree << std::endl;

    for (auto value : insert_values) {
        tree.Delete(value);
        std::cout << "after delete value " << value << ": \n";
        std::cout << tree << std::endl;
    }

    return 0;
}
