import concurrent.futures
import requests
import time
server_port = 8003  # Replace with the actual server port
url = f"http://localhost:{server_port}/output.cgi?0.3"
num_requests = 10

def send_request(url):
    response = requests.get(url)
    return response.text

with concurrent.futures.ThreadPoolExecutor() as executor:
    # Submit the GET requests to the ThreadPoolExecutor
    futures = [executor.submit(send_request, url) for _ in range(20)]
    # time.sleep(0.2)
    # url = f"http://localhost:{server_port}/home.html"
    # futures += [executor.submit(send_request, url) for _ in range(20)]
    # Retrieve the results as they become available
    for future in concurrent.futures.as_completed(futures):
        try:
            result = future.result()
            print(f"Response: {result}")
        except Exception as e:
            print(f"Error occurred: {e}")