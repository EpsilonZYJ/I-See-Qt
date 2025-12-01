#ifndef APPCONFIG_H
#define APPCONFIG_H

#include "QtHeaders.h"

namespace Config {
    const QString ORG_NAME = "ISeeOrg";
    const QString APP_NAME = "I See";

    // API Endpoints
    const QString SUBMIT_URL = "https://api.ppinfra.com/v3/async/seedance-v1-pro-t2v";
    const QString QUERY_URL = "https://api.ppinfra.com/v3/async/task-result";

    // Settings Keys
    const QString KEY_SAVE_PATH = "savePath";
    const QString KEY_API_TOKEN = "apiKey";
}

#endif // APPCONFIG_H