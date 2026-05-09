const API = "http://localhost:8080";
const YEARLY_GOAL = 12;

const months = [
  "January", "February", "March", "April", "May", "June",
  "July", "August", "September", "October", "November", "December"
];

const monthSelect = document.querySelector("#month");
const form = document.querySelector("#bookForm");
const shelf = document.querySelector("#shelf");
const chart = document.querySelector("#chart");
const totalBooks = document.querySelector("#totalBooks");
const averageRating = document.querySelector("#averageRating");
const recommendations = document.querySelector("#recommendations");
const goalText = document.querySelector("#goalText");
const crawlFill = document.querySelector("#crawlFill");
const crawlBug = document.querySelector("#crawlBug");

months.forEach(month => {
  const option = document.createElement("option");
  option.textContent = month;
  option.value = month;
  monthSelect.appendChild(option);
});

form.addEventListener("submit", async event => {
  event.preventDefault();

  const data = new URLSearchParams();
  data.set("title", document.querySelector("#title").value);
  data.set("author", document.querySelector("#author").value);
  data.set("rating", document.querySelector("#rating").value);
  data.set("month", document.querySelector("#month").value);

  await fetch(`${API}/api/books`, {
    method: "POST",
    headers: { "Content-Type": "application/x-www-form-urlencoded" },
    body: data
  });

  form.reset();
  await loadEverything();
});

async function loadEverything() {
  const books = await fetch(`${API}/api/books`)
    .then(response => response.json())
    .catch(() => []);

  const recs = await fetch(`${API}/api/recommendations`)
    .then(response => response.json())
    .catch(() => [
      "Start the C++ backend to wake up the recommendation caterpillar."
    ]);

  renderShelf(books);
  renderStats(books);
  renderChart(books);
  renderGoal(books);
  renderRecommendations(recs);
}

function renderShelf(books) {
  shelf.innerHTML = "";

  if (!books.length) {
    shelf.innerHTML = `<p class="muted">No books yet. The caterpillar is hungry for chapters.</p>`;
    return;
  }

  books.forEach(book => {
    const item = document.createElement("div");
    item.className = "book";
    item.innerHTML = `
      <strong>🐛 ${escapeHtml(book.title)}</strong>
      <small>by ${escapeHtml(book.author)} • finished in ${escapeHtml(book.month)}</small>
      <div class="stars">${"🍃".repeat(book.rating)}${"○".repeat(5 - book.rating)}</div>
    `;
    shelf.appendChild(item);
  });
}

function renderStats(books) {
  totalBooks.textContent = books.length;

  const average = books.length
    ? books.reduce((sum, book) => sum + Number(book.rating), 0) / books.length
    : 0;

  averageRating.textContent = average.toFixed(1);
}

function renderGoal(books) {
  const progress = Math.min(100, Math.round((books.length / YEARLY_GOAL) * 100));

  crawlFill.style.width = `${progress}%`;
  crawlBug.style.left = `calc(${progress}% - 16px)`;

  if (books.length === 0) {
    goalText.textContent = `You are at 0/${YEARLY_GOAL} books. Time to take the first tiny inch.`;
  } else if (books.length < YEARLY_GOAL) {
    goalText.textContent = `You have read ${books.length}/${YEARLY_GOAL} books. Keep inching closer.`;
  } else {
    goalText.textContent = `You reached your ${YEARLY_GOAL}-book goal. Full butterfly behavior unlocked.`;
  }
}

function renderChart(books) {
  chart.innerHTML = "";

  const counts = months.map(month => books.filter(book => book.month === month).length);
  const max = Math.max(1, ...counts);

  months.forEach((month, index) => {
    const wrap = document.createElement("div");
    wrap.className = "bar-wrap";

    const bar = document.createElement("div");
    bar.className = "bar";
    bar.style.height = `${Math.max(8, (counts[index] / max) * 200)}px`;
    bar.title = `${month}: ${counts[index]} book(s) nibbled`;

    const label = document.createElement("div");
    label.className = "bar-label";
    label.textContent = month.slice(0, 3);

    wrap.appendChild(bar);
    wrap.appendChild(label);
    chart.appendChild(wrap);
  });
}

function renderRecommendations(recs) {
  recommendations.innerHTML = "";

  recs.forEach(title => {
    const item = document.createElement("div");
    item.className = "rec";
    item.textContent = `🍃 ${title}`;
    recommendations.appendChild(item);
  });
}

function escapeHtml(text) {
  return String(text).replace(/[&<>"']/g, character => ({
    "&": "&amp;",
    "<": "&lt;",
    ">": "&gt;",
    '"': "&quot;",
    "'": "&#039;"
  }[character]));
}

loadEverything();