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

function postForm(url, params) {
    return fetch(url, {
        method: 'POST',
        headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
        body: new URLSearchParams(params).toString()
    });
}

function toggleFollow(button) {
    const userId = button.dataset.userId;
    const isFollowing = button.textContent.trim().toLowerCase() === 'following';
    const endpoint = isFollowing ? '/unfollow' : '/follow';

    button.disabled = true;

    postForm(endpoint, { user_id: userId })
        .then(response => {
            if (!response.ok) throw new Error('Follow request failed');
            button.textContent = isFollowing ? 'Follow' : 'Following';
            button.classList.toggle('bg-xtext', isFollowing);
            button.classList.toggle('text-xblack', isFollowing);
            button.classList.toggle('border', !isFollowing);
            button.classList.toggle('border-xborder', !isFollowing);
        })
        .catch(() => {
            alert('Could not update follow status. Please try again.');
        })
        .finally(() => {
            button.disabled = false;
        });
}

function likePost(button) {
    const postId = button.dataset.postId;
    const icon = button.querySelector('i');
    const count = button.querySelector('span');
    const isLiked = icon && icon.classList.contains('ph-fill');
    const endpoint = isLiked ? '/unlike' : '/like';

    button.disabled = true;

    postForm(endpoint, { post_id: postId })
        .then(response => {
            if (!response.ok) throw new Error('Like request failed');
            if (icon) icon.classList.toggle('ph-fill', !isLiked);
            if (count) {
                const current = parseInt(count.textContent || '0', 10);
                count.textContent = String(Math.max(0, current + (isLiked ? -1 : 1)));
            }
        })
        .catch(() => {
            alert('Could not update like status. Please try again.');
        })
        .finally(() => {
            button.disabled = false;
        });
}

function requestHubAccess(button) {
    const programme = button.dataset.programme;
    button.disabled = true;
    const originalText = button.textContent;
    button.textContent = 'Requesting...';

    postForm('/request-access', { programme })
        .then(response => {
            if (!response.ok) throw new Error('Access request failed');
            button.textContent = 'Request sent';
        })
        .catch(() => {
            button.textContent = originalText;
            alert('Could not send access request. Please try again.');
        })
        .finally(() => {
            button.disabled = false;
        });
}
