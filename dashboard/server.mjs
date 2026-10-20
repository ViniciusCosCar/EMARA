import { createServer } from 'node:http';
import { readFile } from 'node:fs/promises';

const hostname = '127.0.0.1';
const port = 3000;

const server = createServer(async (req, res) => {
        try {
                const html = await readFile('./index.html', 'utf8');
                res.statusCode = 200;
                res.setHeader('Content-Type', 'text/html');
                res.end(html);
        } catch (err) {
                res.statusCode = 500;
                res.end('Error loading page');
        }
});

server.listen(port, hostname, () => {
        console.log(`Server running at http://${hostname}:${port}/`);
});
