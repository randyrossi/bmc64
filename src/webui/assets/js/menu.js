// The pop-up menu behind each file row's "Actions" button. There is one
// shared menu element on <body>, positioned from the button, rather than a
// dropdown inside the row: the file table sits in a scroll container that
// would clip it.

let menu = null;   // the pop-up element, created on first use
let owner = null;  // the button the menu is currently open for
let openWidth = 0; // window width when it opened

function ensureMenu() {
  if (menu) return;
  menu = document.createElement("div");
  menu.className = "act-menu";
  menu.setAttribute("role", "menu");
  menu.hidden = true;
  menu.addEventListener("keydown", onMenuKey);
  document.body.appendChild(menu);

  document.addEventListener("click", (ev) => {
    if (owner && !menu.contains(ev.target) && !owner.contains(ev.target)) {
      closeMenu();
    }
  });
  document.addEventListener("keydown", (ev) => {
    if (owner && ev.key === "Escape") {
      const button = owner;
      closeMenu();
      button.focus();
    }
  });
  // The menu is positioned once, so anything that moves the button closes
  // it. A phone's address bar showing or hiding only changes the height, so
  // only a change of width (rotation, window resize) counts.
  window.addEventListener("scroll", closeMenu, true);
  window.addEventListener("resize", () => {
    if (window.innerWidth !== openWidth) closeMenu();
  });
  window.addEventListener("hashchange", closeMenu);
}

function onMenuKey(ev) {
  const items = Array.from(menu.querySelectorAll(".act-item"));
  const at = items.indexOf(document.activeElement);
  if (ev.key === "ArrowDown") {
    ev.preventDefault();
    items[(at + 1) % items.length].focus();
  } else if (ev.key === "ArrowUp") {
    ev.preventDefault();
    items[(at - 1 + items.length) % items.length].focus();
  } else if (ev.key === "Tab") {
    closeMenu();
  }
}

export function closeMenu() {
  if (!owner) return;
  owner.setAttribute("aria-expanded", "false");
  owner = null;
  menu.hidden = true;
  menu.textContent = "";
}

// Open the menu under `button` with `items` (or close it if it is already
// open for that button). Each item is { label, run } for an action,
// { label, href } for a download link, and may set `danger`.
export function toggleMenu(button, items) {
  const wasOpen = owner === button;
  closeMenu();
  if (wasOpen || !items.length) return;
  ensureMenu();

  for (const item of items) {
    const el = document.createElement(item.href ? "a" : "button");
    el.className = "act-item" + (item.danger ? " danger" : "");
    el.setAttribute("role", "menuitem");
    el.textContent = item.label;
    if (item.href) {
      el.href = item.href;
      el.setAttribute("download", "");
      // Let the click start the download before the link is removed.
      el.addEventListener("click", () => setTimeout(closeMenu, 0));
    } else {
      el.type = "button";
      el.addEventListener("click", () => {
        closeMenu();
        item.run();
      });
    }
    menu.appendChild(el);
  }

  owner = button;
  openWidth = window.innerWidth;
  button.setAttribute("aria-expanded", "true");
  menu.hidden = false;

  // Right-align under the button, kept inside the window; open upwards when
  // there is no room below.
  const rect = button.getBoundingClientRect();
  const width = menu.offsetWidth;
  const height = menu.offsetHeight;
  const left = Math.max(8, Math.min(rect.right - width,
                                    window.innerWidth - width - 8));
  let top = rect.bottom + 4;
  if (top + height > window.innerHeight - 8) {
    top = Math.max(8, rect.top - height - 4);
  }
  menu.style.left = left + "px";
  menu.style.top = top + "px";
  menu.firstElementChild.focus({ preventScroll: true });
}
