#include <iostream>
#include <algorithm>

class Order {
public:
    int idNumber;
    bool buyOrSell; 
    int shares;
    int limit;
    int entryTime;
    int eventTime;
    Order* nextOrder = nullptr;
    Order* prevOrder = nullptr;
    class Limit* parentLimit = nullptr;
};

class Limit {
public:
    int limitPrice;
    int size = 0;
    int totalVolume = 0;
    Limit* parent = nullptr;
    Limit* leftChild = nullptr;
    Limit* rightChild = nullptr;
    Order* headOrder = nullptr;
    Order* tailOrder = nullptr;

    Limit(int price) : limitPrice(price) {}
};

class SellTree {
public:
    Limit* root = nullptr;

    void AddSellOrder(Order* newOrder) {
        if (!root) {
            root = new Limit(newOrder->limit);
            root->headOrder = root->tailOrder = newOrder;
            newOrder->parentLimit = root;
            root->size++;
            root->totalVolume += newOrder->shares;
            return;
        }

        Limit* current = root;
        Limit* parent = nullptr;

        while (current) {
            parent = current;
            if (newOrder->limit < current->limitPrice)
                current = current->leftChild;
            else if (newOrder->limit > current->limitPrice)
                current = current->rightChild;
            else {
                if (!current->tailOrder) {
                    current->headOrder = current->tailOrder = newOrder;
                }
                else {
                    current->tailOrder->nextOrder = newOrder;
                    newOrder->prevOrder = current->tailOrder;
                    current->tailOrder = newOrder;
                }
                newOrder->parentLimit = current;
                current->size++;
                current->totalVolume += newOrder->shares;
                return;
            }
        }

        Limit* newLimit = new Limit(newOrder->limit);
        newOrder->parentLimit = newLimit;
        newLimit->headOrder = newLimit->tailOrder = newOrder;
        newLimit->size++;
        newLimit->totalVolume += newOrder->shares;

        if (newOrder->limit < parent->limitPrice)
            parent->leftChild = newLimit;
        else
            parent->rightChild = newLimit;
    }

    Limit* findLowestPrice() {
        if (!root)
            return nullptr;

        Limit* node = root;
        while (node && node->leftChild) {
            node = node->leftChild;
        }
        return node;
    }

    void removeLimitNode(Limit* limitNode) {
        if (!limitNode)
            return;

        if (!limitNode->leftChild && !limitNode->rightChild) {
            if (limitNode->parent) {
                if (limitNode->parent->leftChild == limitNode)
                    limitNode->parent->leftChild = nullptr;
                else
                    limitNode->parent->rightChild = nullptr;
            }
            else {
                root = nullptr;
            }
            delete limitNode;
            return;
        }

        Limit* child = limitNode->leftChild ? limitNode->leftChild : limitNode->rightChild;
        if (limitNode->parent) {
            if (limitNode->parent->leftChild == limitNode)
                limitNode->parent->leftChild = child;
            else
                limitNode->parent->rightChild = child;
        }
        else {
            root = child;
        }
        child->parent = limitNode->parent;
        delete limitNode;
    }

    void MatchSellOrder(Order* buyOrder) {
        while (buyOrder->shares > 0) {
            Limit* lowestLimit = findLowestPrice();
            if (!lowestLimit || lowestLimit->limitPrice > buyOrder->limit) {
                break;
            }

            Order* currentOrder = lowestLimit->headOrder;
            while (currentOrder && buyOrder->shares > 0) {
                int tradedShares = std::min(buyOrder->shares, currentOrder->shares);
                buyOrder->shares -= tradedShares;
                currentOrder->shares -= tradedShares;

                if (currentOrder->shares == 0) {
                    if (currentOrder == lowestLimit->headOrder)
                        lowestLimit->headOrder = currentOrder->nextOrder;
                    if (currentOrder == lowestLimit->tailOrder)
                        lowestLimit->tailOrder = currentOrder->prevOrder;

                    if (currentOrder->prevOrder)
                        currentOrder->prevOrder->nextOrder = currentOrder->nextOrder;
                    if (currentOrder->nextOrder)
                        currentOrder->nextOrder->prevOrder = currentOrder->prevOrder;

                    delete currentOrder;
                }

                currentOrder = lowestLimit->headOrder;
            }

            if (!lowestLimit->headOrder) {
                removeLimitNode(lowestLimit);
            }
        }
    }
};

class BuyTree {
    Limit* root=nullptr;

public:
    void AddBuyOrder(Order* newOrder) {
        if (!root) {
            root = new Limit(newOrder->limit);
            root->headOrder = root->tailOrder = newOrder;
            newOrder->parentLimit = root;
            root->size++;
            root->totalVolume += newOrder->shares;
            return;
        }

        Limit* current = root;
        Limit* parent = nullptr;

        while (current) {
            parent = current;
            if (newOrder->limit < current->limitPrice)
                current = current->leftChild;
            else if (newOrder->limit > current->limitPrice)
                current = current->rightChild;
            else {
                if (!current->tailOrder) {
                    current->headOrder = current->tailOrder = newOrder;
                }
                else {
                    current->tailOrder->nextOrder = newOrder;
                    newOrder->prevOrder = current->tailOrder;
                    current->tailOrder = newOrder;
                }
                newOrder->parentLimit = current;
                current->size++;
                current->totalVolume += newOrder->shares;
                return;
            }
        }

        Limit* newLimit = new Limit(newOrder->limit);
        newOrder->parentLimit = newLimit;
        newLimit->headOrder = newLimit->tailOrder = newOrder;
        newLimit->size++;
        newLimit->totalVolume += newOrder->shares;

        if (newOrder->limit < parent->limitPrice)
            parent->leftChild = newLimit;
        else
            parent->rightChild = newLimit;
    }

    Limit* findHighestOrder() {
        if (!root) {
            return nullptr;
        }
        else {
            Limit* node = root;
            while (node && node->rightChild) {
                node = node->rightChild;
            }
            return node;
        }
    }

    void RemoveBuyOrder(Limit* limitNode) {
        if (!limitNode)
            return;

        if (!limitNode->leftChild && !limitNode->rightChild) {
            if (limitNode->parent) {
                if (limitNode->parent->leftChild == limitNode)
                    limitNode->parent->leftChild = nullptr;
                else
                    limitNode->parent->rightChild = nullptr;
            }
            else {
                root = nullptr;
            }
            delete limitNode;
            return;
        }

        Limit* child = limitNode->leftChild ? limitNode->leftChild : limitNode->rightChild;
        if (limitNode->parent) {
            if (limitNode->parent->leftChild == limitNode)
                limitNode->parent->leftChild = child;
            else
                limitNode->parent->rightChild = child;
        }
        else {
            root = child;
        }
        child->parent = limitNode->parent;
        delete limitNode;
    }
    void MatchBuyOrder(Order* sellOrder) {
        while (sellOrder->shares > 0) {
            Limit* highestLimit = findHighestOrder();
            if (!highestLimit || highestLimit->limitPrice < sellOrder->limit) {
                break;
            }

            Order* currentOrder = highestLimit->headOrder;
            while (currentOrder && sellOrder->shares > 0) {
                int tradedShares = std::min(sellOrder->shares, currentOrder->shares);
                sellOrder->shares -= tradedShares;
                currentOrder->shares -= tradedShares;

                if (currentOrder->shares == 0) {
                    if (currentOrder == highestLimit->headOrder)
                        highestLimit->headOrder = currentOrder->nextOrder;
                    if (currentOrder == highestLimit->tailOrder)
                        highestLimit->tailOrder = currentOrder->prevOrder;

                    if (currentOrder->prevOrder)
                        currentOrder->prevOrder->nextOrder = currentOrder->nextOrder;
                    if (currentOrder->nextOrder)
                        currentOrder->nextOrder->prevOrder = currentOrder->prevOrder;

                    delete currentOrder;
                }

                currentOrder = highestLimit->headOrder;
            }

            if (!highestLimit->headOrder) {
                RemoveBuyOrder(highestLimit);
            }
        }
    }

};
class OrderBook {
public:
    BuyTree buy;
    SellTree sell;
    void AddOrder(Order* NewOrder) {
        if (NewOrder->buyOrSell) {
            sell.MatchSellOrder(NewOrder);
            if (NewOrder->shares > 0) {
                buy.AddBuyOrder(NewOrder);
            }
        }
        else {
            buy.MatchBuyOrder(NewOrder);
            if (NewOrder->shares > 0) {
                sell.AddSellOrder(NewOrder);
            }
        }
    }
    void CancelOrder(Order* order) {
        if (!order) return;

        Limit* parentLimit = order->parentLimit;
        if (!parentLimit) return;

        if (order->prevOrder) {
            order->prevOrder->nextOrder = order->nextOrder;
        }
        if (order->nextOrder) {
            order->nextOrder->prevOrder = order->prevOrder;
        }
        if (order == parentLimit->headOrder) {
            parentLimit->headOrder = order->nextOrder;
        }
        if (order == parentLimit->tailOrder) {
            parentLimit->tailOrder = order->prevOrder;
        }
        parentLimit->size--;
        parentLimit->totalVolume -= order->shares;

        if (parentLimit->size == 0) {
            if (order->buyOrSell) {
                buy.RemoveBuyOrder(parentLimit);
            }
            else {
                sell.removeLimitNode(parentLimit);
            }
        }

        order->nextOrder = nullptr;
        order->prevOrder = nullptr;
        order->parentLimit = nullptr;

        delete order;
    }



    void ProcessMatching(Order* incomingOrder) {
        if (incomingOrder->buyOrSell) { 
            sell.MatchSellOrder(incomingOrder);
            if (incomingOrder->shares > 0) {
                buy.AddBuyOrder(incomingOrder);
            }
        }
        else { 
            buy.MatchBuyOrder(incomingOrder);
            if (incomingOrder->shares > 0) {
                sell.AddSellOrder(incomingOrder);
            }
        }
    }


};

int main() {
    // Create the order book
    OrderBook orderBook;

    // Helper function to create and return a new order
    auto createOrder = [](int id, bool buyOrSell, int shares, int limit, int entryTime) {
        Order* order = new Order();
        order->idNumber = id;
        order->buyOrSell = buyOrSell;
        order->shares = shares;
        order->limit = limit;
        order->entryTime = entryTime;
        return order;
        };

    // Add some buy and sell orders
    Order* buyOrder1 = createOrder(1, true, 100, 50, 1); // Buy 100 shares at limit 50
    Order* buyOrder2 = createOrder(2, true, 150, 55, 2); // Buy 150 shares at limit 55
    Order* sellOrder1 = createOrder(3, false, 120, 45, 3); // Sell 120 shares at limit 45
    Order* sellOrder2 = createOrder(4, false, 200, 50, 4); // Sell 200 shares at limit 50

    std::cout << "Adding orders to the order book...\n";
    orderBook.AddOrder(buyOrder1);
    orderBook.AddOrder(buyOrder2);
    orderBook.AddOrder(sellOrder1);
    orderBook.AddOrder(sellOrder2);

    // Adding a new buy order that matches with existing sell orders
    std::cout << "\nProcessing a buy order that matches sell orders...\n";
    Order* matchingBuyOrder = createOrder(5, true, 150, 55, 5); // Buy 150 shares at limit 55
    orderBook.ProcessMatching(matchingBuyOrder);

    // Adding a new sell order that matches with existing buy orders
    std::cout << "\nProcessing a sell order that matches buy orders...\n";
    Order* matchingSellOrder = createOrder(6, false, 100, 45, 6); // Sell 100 shares at limit 45
    orderBook.ProcessMatching(matchingSellOrder);

    // Canceling an order
    std::cout << "\nCanceling an order from the order book...\n";
    orderBook.CancelOrder(buyOrder2);

    // Cleanup dynamically allocated orders
    delete buyOrder1;
    delete buyOrder2;
    delete sellOrder1;
    delete sellOrder2;
    delete matchingBuyOrder;
    delete matchingSellOrder;

    return 0;
}