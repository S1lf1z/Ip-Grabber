#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <curl/curl.h>
#include <json/json.h>

using namespace std;

string getRequest(const string& url) {
    CURL* curl;
    CURLcode res;
    string readBuffer;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, 
            [](void* contents, size_t size, size_t nmemb, string* output) -> size_t {
                size_t totalSize = size * nmemb;
                output->append((char*)contents, totalSize);
                return totalSize;
            }
        );
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << endl;
        }
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();

    return readBuffer;
}
void processIPRanges(const string& jsonData, const string& asn) {
    Json::CharReaderBuilder readerBuilder;
    Json::Value root;
    istringstream stream(jsonData);
    string errs;

    if (Json::parseFromStream(readerBuilder, stream, &root, &errs)) {
        const Json::Value ipv4Prefixes = root["data"]["ipv4_prefixes"];

        ofstream outFile(asn + "_IPS.txt", ios::app);

        for (const auto& prefix : ipv4Prefixes) {
            string ip = prefix["ip"].asString();
            int cidr = prefix["cidr"].asInt();
            string ipRange = ip + "/" + to_string(cidr);
            outFile << ipRange << endl;
        }

        outFile.close();
    } else {
        cerr << "Error al analizar el JSON: " << errs << endl;
    }
}

int main() {
    cout << "By Self$" << endl;
    string asn;
    cout << "ASN >> : ";
    cin >> asn;

    string url = "https://api.bgpview.io/asn/" + asn + "/prefixes";
    string response = getRequest(url);

    processIPRanges(response, asn);

    return 0;
}
