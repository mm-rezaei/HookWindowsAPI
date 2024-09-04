#pragma once

#include <functional>
#include <string>
#include <vector>
#include <mutex>

using namespace std;

class Notifier
{
public:

    using NotificationCallback = function<void(const wstring&)>;
    using ErrorCallback = function<void(const wstring&)>;

    Notifier() = default;

    // Register a callback for notifications
    void RegisterNotificationCallback(NotificationCallback callback)
    {
        lock_guard<mutex> lock(notificationMutex);

        notificationCallbacks.push_back(callback);
    }

    // Register a callback for errors
    void RegisterErrorCallback(ErrorCallback callback)
    {
        lock_guard<mutex> lock(errorMutex);

        errorCallbacks.push_back(callback);
    }

    // Notify all registered listeners about a message
    void Notify(const wstring& message)
    {
        lock_guard<mutex> lock(notificationMutex);

        for (const auto& callback : notificationCallbacks)
        {
            callback(L"Notification: " + message);
        }
    }

    // Notify all registered listeners about an error
    void NotifyError(const wstring& errorMessage)
    {
        lock_guard<mutex> lock(errorMutex);

        for (const auto& callback : errorCallbacks)
        {
            callback(L"Error: " + errorMessage);
        }
    }

private:

    vector<NotificationCallback> notificationCallbacks;
    vector<ErrorCallback> errorCallbacks;
    mutable mutex notificationMutex;
    mutable mutex errorMutex;
};