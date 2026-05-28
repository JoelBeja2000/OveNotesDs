const fs = require('fs');

async function checkReplies() {
    try {
        const response = await fetch('https://www.reddit.com/r/nds/comments/1tpiorg.json', {
            headers: {
                'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36'
            }
        });
        if (!response.ok) {
            console.error('Failed to fetch:', response.statusText);
            return;
        }
        const data = await response.json();
        const comments = data[1].data.children;
        const myUsername = 'FokaMotorista';
        const pending = [];

        function traverse(comment) {
            if (comment.kind !== 't1') return;
            const cData = comment.data;
            const author = cData.author;
            const body = cData.body;
            const id = cData.id;
            const permalink = cData.permalink;

            // Check if this comment has replies
            let hasMyReply = false;
            if (cData.replies && cData.replies.data && cData.replies.data.children) {
                for (const reply of cData.replies.data.children) {
                    if (reply.data && reply.data.author === myUsername) {
                        hasMyReply = true;
                        break;
                    }
                }
            }

            if (author !== myUsername && !hasMyReply && author !== '[deleted]' && author !== 'AutoModerator') {
                pending.push({
                    id,
                    author,
                    body,
                    permalink: `https://www.reddit.com${permalink}`
                });
            }

            if (cData.replies && cData.replies.data && cData.replies.data.children) {
                for (const reply of cData.replies.data.children) {
                    traverse(reply);
                }
            }
        }

        for (const item of comments) {
            traverse(item);
        }

        pending.slice(0, 15).forEach((p, index) => {
            console.log(`--- COMMENT ${index + 1} ---`);
            console.log(`Author: ${p.author} (${p.id})`);
            console.log(`Link: ${p.permalink}`);
            console.log(`Body:\n${p.body}\n`);
        });
    } catch (err) {
        console.error(err);
    }
}

checkReplies();
