+#include <iomanip>
+#include <iostream>
+#include <limits>
+#include <sstream>
+#include <string>
+#include <unordered_map>
+
+struct Account {
+  std::string owner;
+  long long balance_cents = 0;
+};
+
+class Bank {
+ public:
+  bool CreateAccount(const std::string &owner, const std::string &account_id,
+                     long long initial_deposit_cents) {
+    if (accounts_.count(account_id) > 0 || owner.empty()) {
+      return false;
+    }
+    accounts_.emplace(account_id, Account{owner, initial_deposit_cents});
+    return true;
+  }
+
+  bool Deposit(const std::string &account_id, long long amount_cents) {
+    auto account = FindAccount(account_id);
+    if (!account || amount_cents <= 0) {
+      return false;
+    }
+    account->balance_cents += amount_cents;
+    return true;
+  }
+
+  bool Withdraw(const std::string &account_id, long long amount_cents) {
+    auto account = FindAccount(account_id);
+    if (!account || amount_cents <= 0 || account->balance_cents < amount_cents) {
+      return false;
+    }
+    account->balance_cents -= amount_cents;
+    return true;
+  }
+
+  const Account *GetAccount(const std::string &account_id) const {
+    auto it = accounts_.find(account_id);
+    if (it == accounts_.end()) {
+      return nullptr;
+    }
+    return &it->second;
+  }
+
+  void ListAccounts() const {
+    if (accounts_.empty()) {
+      std::cout << "No accounts found.\n";
+      return;
+    }
+
+    std::cout << "\nExisting Accounts\n";
+    for (const auto &entry : accounts_) {
+      const auto &account = entry.second;
+      std::cout << "- [" << entry.first << "] " << account.owner << " | Balance: "
+                << FormatCurrency(account.balance_cents) << "\n";
+    }
+  }
+
+ private:
+  static std::string FormatCurrency(long long cents) {
+    std::ostringstream stream;
+    stream << "$" << (cents / 100) << '.' << std::setw(2) << std::setfill('0')
+           << std::abs(cents % 100);
+    return stream.str();
+  }
+
+  Account *FindAccount(const std::string &account_id) {
+    auto it = accounts_.find(account_id);
+    if (it == accounts_.end()) {
+      return nullptr;
+    }
+    return &it->second;
+  }
+
+  std::unordered_map<std::string, Account> accounts_;
+};
+
+long long ReadAmount() {
+  double amount = 0.0;
+  std::cin >> amount;
+  if (!std::cin) {
+    std::cin.clear();
+    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
+    return 0;
+  }
+  return static_cast<long long>(amount * 100.0 + (amount >= 0 ? 0.5 : -0.5));
+}
+
+int main() {
+  Bank bank;
+  while (true) {
+    std::cout << "\nBanking Management System\n"
+              << "1. Create account\n"
+              << "2. Deposit\n"
+              << "3. Withdraw\n"
+              << "4. View account\n"
+              << "5. List accounts\n"
+              << "6. Exit\n"
+              << "Select an option: ";
+
+    int choice = 0;
+    std::cin >> choice;
+    if (!std::cin) {
+      std::cin.clear();
+      std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
+      std::cout << "Invalid input. Try again.\n";
+      continue;
+    }
+
+    if (choice == 6) {
+      std::cout << "Goodbye!\n";
+      break;
+    }
+
+    std::string account_id;
+    std::string owner;
+
+    switch (choice) {
+      case 1: {
+        std::cout << "Enter account ID: ";
+        std::cin >> account_id;
+        std::cout << "Enter owner name: ";
+        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
+        std::getline(std::cin, owner);
+        std::cout << "Enter initial deposit: ";
+        long long deposit = ReadAmount();
+        if (deposit < 0) {
+          std::cout << "Initial deposit cannot be negative.\n";
+          break;
+        }
+        if (bank.CreateAccount(owner, account_id, deposit)) {
+          std::cout << "Account created.\n";
+        } else {
+          std::cout << "Failed to create account.\n";
+        }
+        break;
+      }
+      case 2: {
+        std::cout << "Enter account ID: ";
+        std::cin >> account_id;
+        std::cout << "Enter deposit amount: ";
+        long long deposit = ReadAmount();
+        if (bank.Deposit(account_id, deposit)) {
+          std::cout << "Deposit successful.\n";
+        } else {
+          std::cout << "Deposit failed.\n";
+        }
+        break;
+      }
+      case 3: {
+        std::cout << "Enter account ID: ";
+        std::cin >> account_id;
+        std::cout << "Enter withdrawal amount: ";
+        long long withdrawal = ReadAmount();
+        if (bank.Withdraw(account_id, withdrawal)) {
+          std::cout << "Withdrawal successful.\n";
+        } else {
+          std::cout << "Withdrawal failed.\n";
+        }
+        break;
+      }
+      case 4: {
+        std::cout << "Enter account ID: ";
+        std::cin >> account_id;
+        const Account *account = bank.GetAccount(account_id);
+        if (!account) {
+          std::cout << "Account not found.\n";
+        } else {
+          std::cout << "Owner: " << account->owner
+                    << " | Balance: $" << (account->balance_cents / 100) << '.'
+                    << std::setw(2) << std::setfill('0')
+                    << std::abs(account->balance_cents % 100) << "\n";
+        }
+        break;
+      }
+      case 5:
+        bank.ListAccounts();
+        break;
+      default:
+        std::cout << "Invalid option.\n";
+        break;
+    }
+  }
+  return 0;
+}
 
EOF
)
