async function login() {
    const res = await fetch("/login", {
        method: "POST",
        headers: {
            "Content-Type": "application/json"
        },
        body: JSON.stringify({
            username: "user",
            password: "pass"
        })
    });

    const data = await res.json();

    if (!data.token) {
        alert("Login failed");
        return;
    }

    // ✅ STORE TOKEN
    localStorage.setItem("token", data.token);

    // redirect
    window.location.href = "/";
}