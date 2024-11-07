#include "ConsoleManager.h"
#include "NetworkManager.h"

#include <thread>


 struct ListNode {
    int val;
    ListNode* next;
    ListNode() : val(0), next(nullptr) {}
    ListNode(int x) : val(x), next(nullptr) {}
    ListNode(int x, ListNode* next) : val(x), next(next) {}
    
};

 ListNode* mergeTwoLists(ListNode* list1, ListNode* list2) {
     ListNode* result = nullptr;
     ListNode* next_node = nullptr;
     ListNode** current_list = nullptr;
     ListNode* result_last = nullptr;
     while (list1 != nullptr || list2 != nullptr) {
         if (list1 != nullptr && list2 != nullptr) {
             current_list = (list1->val <= list2->val) ? &list1 : &list2;
         }
         else {
             current_list = ((list1 != nullptr) ? &list1 : &list2);
         }
         next_node = *current_list;
         *current_list = (*current_list)->next;
         next_node->next = nullptr;

         if (result == nullptr) {
             result = next_node;
             result_last = result;
         }
         else {
             result_last->next = next_node;
             result_last = result_last->next;
         }
     }
     return result;
 }

     ListNode* mergeKLists(std::vector<ListNode*>& lists) {
         std::vector<ListNode*> tmp{};
         if (lists.size() == 0) {
             return nullptr;
         }
         while (lists.size() != 1) {
             for (int i = 0; i < lists.size() - lists.size() % 2; i += 2) {
                 auto list1 = lists[i];
                 auto list2 = lists[i + 1];
                 tmp.push_back(mergeTwoLists(list1, list2));
             }
             if (lists.size() % 2) {
                 tmp.push_back(lists.back());
             }
             lists = tmp;
             tmp.clear();
         }
         return lists.back();
     }

     ListNode* mergeKLists2(std::vector<ListNode*>& lists) {
         if (lists.size() == 0) {
             return nullptr;
         }
         while (lists.size() != 1) {
             int i = lists.size() - 2;
             auto list1 = lists[i];
             auto list2 = lists[i + 1];
             lists[i] = mergeTwoLists(list1, list2);
             lists.pop_back();
         }
         return lists.back();
     }

#include <map>
     ListNode* mergeKLists3(std::vector<ListNode*>& lists) {
         if (lists.size() == 0) {
             return nullptr;
         }
         std::multimap<int, ListNode*> tree{};
         for (auto list : lists) {
             if (list != nullptr) {
                 tree.insert({ list->val, list });
             }
         }
         ListNode* result{};
         ListNode* last_node{};
         ListNode* next_node{};
         while (tree.size() != 0) {
             auto next_node_it = tree.begin();
             next_node = (*next_node_it).second;
             tree.erase(next_node_it);
             if (next_node->next != nullptr) {
                 tree.insert({ next_node->next->val, next_node->next });
             }
             next_node->next = nullptr;

             if (result == nullptr) {
                 result = next_node;
                 last_node = result;
             }
             else {
                 last_node->next = next_node;
                 last_node = last_node->next;
             }
             
         }
         return result;
     }

#include<queue>

     struct Compare {
         bool operator()(ListNode* a, ListNode* b) {
             return a->val > b->val;
         }
     };


         ListNode* mergeKLists4(std::vector<ListNode*>& lists) {
             if (lists.empty()) {
                 return nullptr;
             }

             std::priority_queue<ListNode*, std::vector<ListNode*>, Compare> pq;

             // Инициализация кучи начальными элементами каждого списка
             for (auto list : lists) {
                 if (list != nullptr) {
                     pq.push(list);
                 }
             }

             ListNode* result = nullptr;
             ListNode* last_node = nullptr;

             // Объединение всех списков
             while (!pq.empty()) {
                 ListNode* next_node = pq.top();
                 pq.pop();

                 if (next_node->next != nullptr) {
                     pq.push(next_node->next);
                 }

                 if (result == nullptr) {
                     result = next_node;
                     last_node = result;
                 }
                 else {
                     last_node->next = next_node;
                     last_node = last_node->next;
                 }
             }

             return result;
         }



#include <iostream>
#include <string>

         const int MOD = 1000000007;

         int recCase(const std::string& sequence, std::string stack, int pos) {
             if (pos == sequence.length()) {
                 return stack.empty() ? 1 : 0;
             }

             switch (sequence[pos]) {
             case '(':
                 return recCase(sequence, stack + "(", pos + 1);
             case ')':
                 if (stack.empty() || stack.back() != '(') {
                     return 0;
                 }
                 return recCase(sequence, stack.substr(0, stack.size() - 1), pos + 1);
             case '[':
                 return recCase(sequence, stack + "[", pos + 1);
             case ']':
                 if (stack.empty() || stack.back() != '[') {
                     return 0;
                 }
                 return recCase(sequence, stack.substr(0, stack.size() - 1), pos + 1);
             case '{':
                 return recCase(sequence, stack + "{", pos + 1);
             case '}':
                 if (stack.empty() || stack.back() != '{') {
                     return 0;
                 }
                 return recCase(sequence, stack.substr(0, stack.size() - 1), pos + 1);
             case '?':
                 int sum = 0;
                 if (!stack.empty()) {
                     sum = (sum + recCase(sequence, stack.substr(0, stack.size() - 1), pos + 1)) % MOD;
                 }
                 sum += (sum + recCase(sequence, stack + "(", pos + 1)) % MOD;
                 sum += (sum + recCase(sequence, stack + "[", pos + 1)) % MOD;
                 sum += (sum + recCase(sequence, stack + "{", pos + 1)) % MOD;
                 return sum;
             }
             return 0;
         }

         int longestSubarray(std::vector<int> nums) {
             std::vector<std::vector<int>> sequences_length{};
             std::vector<int> tmp{};
             int start = -1;

             int i = 0;
             bool flag = 0; //is more then one zero between sequnces of 1
             while (i < nums.size()) {
                 switch (nums[i]) {
                 case 0:
                     if (start != -1) {
                         tmp.push_back(i - start);
                         start = -1;
                     }
                     if (flag == 1 && tmp.size() > 0) {
                         sequences_length.push_back(tmp);
                         tmp.clear();
                     }
                     flag = 1;
                     break;

                 case 1:
                     flag = 0;
                     if (start == -1) {
                         start = i;
                     }
                     break;
                 }
                 i++;
             }
             if (start != -1) {
                 tmp.push_back(i - start);
             }
             if (tmp.size()) {
                 sequences_length.push_back(tmp);
             }
             /*
                 1. лучшая последовательность это две между которыми один ноль 111101111
                 2. весь массив одна последовательность 111111111111
                 3. лучшая последовательность между двумя 000111110000
             */
             int max = -1;
             for (const auto& seq : sequences_length) {
                 if (seq.size() == 1) {
                     if (seq[0] - 1 > max) {
                         max = seq[0];
                         if (max == nums.size()) {
                             max--;
                         }
                     }
                 }
                 else {
                     for (int i = 0; i < seq.size() - 1; i++) {
                         int length = seq[i] + seq[i + 1];
                         if (length > max) {
                             max = length;
                         }
                     }
                 }
             }
             if (max == -1) {
                 max = 0;
             }
             return max;
         }
         int longestSubarray2(std::vector<int> nums) {

             int a = nums.size();
             int i = 0;
             int ans = 0;


             while (i < a) {
                 int j = i - 1;
                 int k = i + 1;
                 int count = 0;

                 if (nums[i] == 0) {
                     while (j >= 0 && nums[j] == 1) {

                         count++;
                         j--;


                     }
                     while (k < a && nums[k] == 1) {

                         count++;
                         k++;

                     }
                     ans = max(ans, count);
                     i = k;
                 }
                 else if (nums[i] == 1) {
                     i++;
                     if (i == a) {
                         ans = a - 1;
                     }
                 }

                 else {
                     i++;
                 }

             }

             return ans;
         }

int main() {
    /*{
        std::vector<int> test;
        for (int i = 0; i < 10000000; i++) {
            test.push_back(rand() % 2);
        }

        auto start = std::chrono::high_resolution_clock::now();
        std::cout << longestSubarray(test) << ' ';



        auto end = std::chrono::high_resolution_clock::now();
        std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        return 0;
    }
    std::vector<ListNode*> test{};
    for (int i = 0; i < 100000; i++) {
        std::vector<int> tmp{};
        auto size = rand() % 501;

        auto val = (rand() % 10001) * ((rand() % 2) ? -1 : 1);
        
        for (int j = 0; j < size; j++) {
            val = (rand() % 10001) * ((rand() % 2) ? -1 : 1);
            tmp.push_back(val);
        }

        
        std::sort(tmp.begin(), tmp.end());
        ListNode* list{};
        ListNode* current_node{};
        for (int j = 0; j < size; j++) {
            if (list == nullptr) {
                list = new ListNode(tmp[j]);
                current_node = list;
            }
            else {
                current_node->next = new ListNode(tmp[j]);
                current_node = current_node->next;
            }
        }
        test.push_back(list);
    }
    auto start = std::chrono::high_resolution_clock::now();

    auto res = mergeKLists4(test);
    
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    exit(123);
    std::cout << res->next;*/
	//for (int i = 500; i < 10000; i++) {
	//	int port_count = i;
	//	constexpr int start_port_number = 20000;
	//	ConsoleManager console_manager(port_count, ConsoleManager::DisplayAll);

	//	NetworkManager network_manager(port_count, start_port_number, console_manager.GetMutex());
	//	network_manager.SetPortPairs(console_manager.GetPortPairs());
	//	console_manager.InitEntries(network_manager.GetOpenedSocketsPorts());
	//	std::thread network_manager_thread(&NetworkManager::ListenSockets, &network_manager);
	//	network_manager_thread.detach();
	//	//network_manager_thread.~thread();
	//	std::thread console_manager_thread(&ConsoleManager::StartConsoleUpdateLoop, &console_manager);
	//	console_manager_thread.detach();
	//	/*std::thread CM_th(&ConsoleManager::StartConsoleUpdateLoop, &console_manager);
	//	CM_th.detach();*/
	//	Sleep(300);
	//	network_manager.StopListenSockets();
	//	console_manager.StopConsoleUpdateLoop();
	//	Sleep(50);
	//}
	constexpr int port_count = 100;
	constexpr int start_port_number = 20000;
	ConsoleManager console_manager(port_count, ConsoleManager::DisplayAll);

	NetworkManager network_manager(port_count, start_port_number, console_manager.GetMutex());
	network_manager.SetPortPairs(console_manager.GetPortPairs());
	console_manager.InitEntries(network_manager.GetOpenedSocketsPorts());
	std::thread network_manager_thread(&NetworkManager::ListenSockets, &network_manager);
	network_manager_thread.detach();
	//network_manager_thread.~thread();
	console_manager.StartConsoleUpdateLoop();
	/*std::thread CM_th(&ConsoleManager::StartConsoleUpdateLoop, &console_manager);
	CM_th.detach();*/
	return 0;
}























