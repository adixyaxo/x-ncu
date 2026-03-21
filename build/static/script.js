document.addEventListener('DOMContentLoaded', () => {

    // --- 1. Dropdown Navigation Logic ---
    const dropdownToggles = document.querySelectorAll('.dropdown-toggle');
    dropdownToggles.forEach(toggle => {
        toggle.addEventListener('click', () => {
            const caret = toggle.querySelector('i');
            const content = toggle.nextElementSibling;

            // Rotate the caret icon (-90 degrees points it right)
            caret.classList.toggle('-rotate-90');

            // Toggle the visibility of the link list
            content.classList.toggle('hidden');
        });
    });

    // --- 2. Active Route Highlighting ---
    const navLinks = document.querySelectorAll('.nav-link');
    function handleRoute() {
        const hash = window.location.hash || '#/';
        navLinks.forEach(link => {
            if (link.getAttribute('href') === hash) {
                link.classList.add('active');
            } else {
                link.classList.remove('active');
            }
        });
        console.log(`Navigating to: ${hash}`);
    }
    window.addEventListener('hashchange', handleRoute);
    handleRoute(); // Initial load run

    // --- 3. Feed Tab Switching (For You / Following) ---
    const tabs = document.querySelectorAll('.feed-tab');
    tabs.forEach(tab => {
        tab.addEventListener('click', (e) => {
            // Remove active states from all
            tabs.forEach(t => {
                t.classList.remove('text-white', 'font-bold');
                t.classList.add('text-xmuted');
                const underline = t.querySelector('.tab-underline');
                if (underline) underline.classList.add('hidden');
            });

            // Add active state to clicked tab
            e.currentTarget.classList.remove('text-xmuted');
            e.currentTarget.classList.add('text-white', 'font-bold');
            const targetUnderline = e.currentTarget.querySelector('.tab-underline');
            if (targetUnderline) targetUnderline.classList.remove('hidden');
        });
    });

    // --- 4. Mock Post Button Action ---
    const postBtn = document.getElementById('post-btn');
    const postInput = document.getElementById('post-input');

    if (postBtn && postInput) {
        postBtn.addEventListener('click', () => {
            if (postInput.value.trim() !== '') {
                console.log('Sending to C++ backend:', postInput.value);
                postInput.value = ''; // Clear input
                // Implement C++ Crow fetch() request here
            }
        });
    }
});