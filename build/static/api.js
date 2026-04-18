// api.js

function getToken() {
    return localStorage.getItem("token"); // or cookie later
}

async function apiFetch(url, options = {}) {
    const token = getToken();

    // default headers
    options.headers = {
        ...(options.headers || {}),
        "Content-Type": "application/json",
        "Authorization": token ? "Bearer " + token : ""
    };

    try {
        const response = await fetch(url, options);

        if (response.status === 401) {
            // token invalid / expired
            alert("Session expired. Please login again.");
            window.location.href = "/login";
            return;
        }

        return response.json();

    } catch (err) {
        console.error("API Error:", err);
    }
}