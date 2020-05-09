// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

namespace media
{

namespace eme
{

using MediaKeyStatusPair = std::tuple<std::vector<uint8_t>, MF_MEDIAKEY_STATUS>;

// Based on the MediaKeySession interface in the EME specification:
// https://www.w3.org/TR/2017/REC-encrypted-media-20170918/#dom-mediakeysession
class MediaKeySession
{
public:
    MediaKeySession(IMFContentDecryptionModule* cdm, MF_MEDIAKEYSESSION_TYPE sessionType);
    virtual ~MediaKeySession();


    // A unique string identifier generated by the CDM that can be used by the application
    // to identify session objects.
    // https://www.w3.org/TR/2017/REC-encrypted-media-20170918/#dom-mediakeysession-sessionid
    wil::unique_cotaskmem_string sessionId();

    // The expiration time for all key(s) in the session, or NaN if no such time exists or
    // if the license explicitly never expires, as determined by the CDM.
    // https://www.w3.org/TR/2017/REC-encrypted-media-20170918/#dom-mediakeysession-expiration
    // expiration is based on https://tc39.es/ecma262/#sec-time-values-and-time-range
    double expiration();

    // A reference to an array of key IDs known to the session to the current status
    // of the associated key. Each entry MUST have a unique key ID.
    // https://www.w3.org/TR/2017/REC-encrypted-media-20170918/#dom-mediakeysession-keystatuses
    std::vector<MediaKeyStatusPair> keyStatuses();

    // Loads the data stored for the specified session into this object.
    // https://www.w3.org/TR/2017/REC-encrypted-media-20170918/#dom-mediakeysession-load
    bool load(const std::wstring& sessionId);

    // Callback registration methods
    using KeyMessageCB = std::function<void(MF_MEDIAKEYSESSION_MESSAGETYPE, gsl::span<uint8_t>, LPCWSTR)>;
    void onmessage(KeyMessageCB&& keyMessageCB);

    using KeyStatusChangedCB = std::function<void()>;
    void onkeystatuseschange(KeyStatusChangedCB&& keyStatusChangedCB);
    
    // Generates a license request based on the initData. A message of type "license-request" or
    // "individualization-request" will always be queued if the algorithm succeeds
    // and the promise is resolved.
    // Structure for init data is based on initDataType
    // https://www.w3.org/TR/2017/REC-encrypted-media-20170918/#dom-mediakeysession-generaterequest
    // https://www.w3.org/TR/eme-initdata-registry/
    void generateRequest(const std::wstring& initDataType, std::vector<uint8_t>& initData);

    // Provides messages, including licenses, to the CDM.
    // https://www.w3.org/TR/2017/REC-encrypted-media-20170918/#dom-mediakeysession-update
    void update(gsl::span<uint8_t> responseData);

    // Indicates that the application no longer needs the session and the CDM should release any resources
    // associated with the session and close it. Persisted data should not be released or cleared.
    // https://www.w3.org/TR/2017/REC-encrypted-media-20170918/#dom-mediakeysession-close
    void close();

    // Removes all license(s) and key(s) associated with the session. For persistent session types,
    // other session data will be cleared as defined for each session type once a
    // release message acknowledgment is processed by Update().
    // https://www.w3.org/TR/2017/REC-encrypted-media-20170918/#dom-mediakeysession-remove
    void remove();

private:
    winrt::com_ptr<IMFContentDecryptionModuleSession> m_cdmSession;
    winrt::com_ptr<IMFContentDecryptionModuleSessionCallbacks> m_sessionCallbacks;
    
    // Callbacks
    KeyMessageCB m_keyMessageCB;
    KeyStatusChangedCB m_keyStatusChangedCB;
    void OnKeyMessage(MF_MEDIAKEYSESSION_MESSAGETYPE messageType, gsl::span<uint8_t> message, LPCWSTR destinationUrl);
    void OnKeyStatusChanged();
};

}

}