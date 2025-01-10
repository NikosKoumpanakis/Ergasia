#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>

#define NC 5  // Number of customers
#define NUM_PRODUCTS 20  // Number of products

// Struct to represent a product
struct product {
    char description[100];
    float price;
    int item_count;
    int total_orders;
    int successful_orders;
    int failed_orders;
};

struct product products[NUM_PRODUCTS];

// Function prototypes
void process_order(int product_index, int *sold_count, int *failed_orders);
void init_products();
void generate_report(int total_orders, int total_successful, int total_failed, float total_revenue);

int main() {
    init_products();  
    pid_t pid;
    int total_orders = 0, total_successful = 0, total_failed = 0;
    float total_revenue = 0.0;

    for (int i = 0; i < NC; i++) {
        int pipe_order[2], pipe_response[2];
        pipe(pipe_order);
        pipe(pipe_response);

        pid = fork();

        if (pid == 0) {  // Customer Process
            close(pipe_order[0]);
            close(pipe_response[1]);

            srand(time(NULL) + getpid());  // ensure that you get random values
            for (int j = 0; j < 10; j++) {
                int product_index = rand() % NUM_PRODUCTS; // random product from 0-20
                write(pipe_order[1], &product_index, sizeof(product_index)); // send to server process each order of each customer
                sleep(1);  // Wait 1 second between orders
                char response[100];
                read(pipe_response[0], response, sizeof(response)); // Get each reult from each order from the sever process
                printf("Customer %d: %s\n", i + 1, response);
            }

            close(pipe_order[1]);
            close(pipe_response[0]);
            exit(0);
        } else {  //Server Process
            close(pipe_order[1]);
            close(pipe_response[0]);

            int sold_count = 0, failed_orders = 0; // initialize sold and failed order counters
            for (int j = 0; j < 10; j++) {
                int product_index;
                read(pipe_order[0], &product_index, sizeof(product_index));
                process_order(product_index, &sold_count, &failed_orders);

                char response[100];
                if (products[product_index].item_count > 0) {
                    snprintf(response, sizeof(response), "Order for %s: Success", products[product_index].description);
                    total_revenue += products[product_index].price;
                } else {
                    snprintf(response, sizeof(response), "Order for %s: Failed", products[product_index].description);
                }
                write(pipe_response[1], response, sizeof(response));
                sleep(1);  // Processing delay
            }

            total_orders += 10;
            total_successful += sold_count;
            total_failed += failed_orders;

            close(pipe_order[0]);
            close(pipe_response[1]);
        }
    }

    for (int i = 0; i < NC; i++) {
        wait(NULL);  // Wait for all customer processes to finish before ending
    }

    printf("\n--- All orders processed. Generating report ---\n");
    generate_report(total_orders, total_successful, total_failed, total_revenue);

    return 0;
}

void process_order(int product_index, int *sold_count, int *failed_orders) {
    products[product_index].total_orders++;
    if (products[product_index].item_count > 0) {
        products[product_index].item_count--;
        products[product_index].successful_orders++;
        (*sold_count)++;
    } else {
        products[product_index].failed_orders++;
        (*failed_orders)++;
    }
}

void init_products() {
    for (int i = 0; i < NUM_PRODUCTS; i++) {
        sprintf(products[i].description, "Product %d", i + 1);
        products[i].price = 13.0 + i * 5.0;  //Random prices for products
        products[i].item_count = 2;
        products[i].total_orders = 0;
        products[i].successful_orders = 0;
        products[i].failed_orders = 0;
    }
}

void generate_report(int total_orders, int total_successful, int total_failed, float total_revenue) {
    printf("\n--- Sales Report ---\n");
    printf("\n--- Summary ---\n");
    printf("Total Orders: %d\n", total_orders);
    printf("Total Successful Orders: %d\n", total_successful);
    printf("Total Failed Orders: %d\n", total_failed);
    printf("Total Revenue: %.2f\n", total_revenue);
}

