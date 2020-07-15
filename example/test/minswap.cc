#include <vector>

using namespace std;

// Complete the minimumSwaps function below.
int minimumSwaps(vector<int> arr) {
    int go = 0;
    int swaps = 0;
    for (int i = 0; i < arr.size();) {
        if (i == arr[i] - 1) {
            ++i;
            continue;
        }
        int t = arr[arr[i] - 1];
        arr[arr[i] - 1] = arr[i];
        arr[i] = t;
        ++swaps; 

        for (int j = 0; j < arr.size(); ++j) {
          printf("%i ", arr[j]);
        }
        printf("\n");
        ++go;
        if (go == 10) break;
    }
    return swaps;
}


int
main(int argc, char** argv)
{
  vector<int> arr = {4, 3, 1, 2};
  minimumSwaps(arr);
  return 0;
}
