if (confirm('Open dialog for testing?'))
    chrome.extension.sendRequest({type:'request_password'});
