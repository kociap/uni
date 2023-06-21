// /usr/local/lib/node_modules/mongodb

'use strict';
(() => {
  const express = require('express');
  const jwt = require('jsonwebtoken');
  const bcrypt = require('bcrypt');

  function generateJWT(user) {
    return jwt.sign({ userId: user.id }, 'secret-key', { expiresIn: '1h' });
  }

  function verifyJWT(req, res, next) {
    console.log("[INFO] authorisation verification headers " + JSON.stringify(req.headers));
    const token = req.headers['authorisation'];
    if (!token) {
      return res.status(401).send('unauthorised');
    }
    jwt.verify(token, 'secret-key', (err, decoded) => {
      if (err) {
        console.log("[INFO] JWT Token invalid " + JSON.stringify(err));
        return res.status(401).send('unauthorised');
      }
      req.userId = decoded.userId;
      next();
    });
  }

  class Database {
    #id = 0;
    #objects = [];

    getAll() {
      return this.#objects;
    }

    getByKey(key, value) {
      let result = this.#objects.find(v => v[key] == value);
      if (result !== undefined) {
        return result;
      } else {
        return null;
      }
    }

    getById(id) {
      return this.getByKey("id", id);
    }

    create(object) {
      object.id = this.#id;
      this.#id += 1;
      this.#objects.push(object);
    }

    update(id, properties) {
      let value = this.getById(id);
      if (value === null) {
        return;
      }

      for (const [k, v] of Object.entries(properties)) {
        if (k === "id") {
          continue;
        }

        value[k] = v;
      }
    }

    remove(id) {
      this.#objects = this.#objects.flter(object => object.id !== id);
    }
  }

  const notesDB = new Database();
  const usersDB = new Database();

  const app = express();
  app.use(express.json());

  app.get('/note', verifyJWT, (req, res) => {
    const notes = notesDB.getAll();
    res.send(notes);
  });

  app.get('/note/:id', verifyJWT, (req, res) => {
    const note = notesDB.getById(req.params.id);
    res.send(note);
  });

  app.post('/note', verifyJWT, (req, res) => {
    const note = {
      title: req.body.title,
      content: req.body.content
    };
    notesDB.create(note);
    res.send(note);
  });

  app.put('/note/:id', verifyJWT, (req, res) => {
    const note = {
      title: req.body.title,
      content: req.body.content
    };
    notesDB.update(req.params.id, note);
    res.send(note);
  });

  app.delete('/note/:id', verifyJWT, (req, res) => {
    notesDB.remove(req.params.id);
    res.send();
  });

  app.post('/login', async (req, res) => {
    const user = usersDB.getByKey("username", req.body.username);
    if (!user) {
      return res.status(404).send('user not found');
    }

    const validPassword = await bcrypt.compare(req.body.password, user.password);
    if (!validPassword) {
      return res.status(401).send('invalid password');
    }

    console.log("[INFO] logged user " + JSON.stringify(user));

    const token = generateJWT(user);
    console.log("[INFO] issuing token \"" + token + "\"");
    res.send(token);
  });

  app.post('/register', async (req, res) => {
    console.log("[INFO] registering user " + JSON.stringify(req.body));

    const existingUser = usersDB.getByKey("username", req.body.username);
    if (existingUser) {
      return res.status(400).send('username already exists');
    }

    if (!req.body.password) {
      return res.status(406).send("missing password");
    }


    const salt = await bcrypt.genSalt(10);
    const hashedPassword = await bcrypt.hash(req.body.password, salt);

    const user = {
      username: req.body.username,
      password: hashedPassword
    };
    usersDB.create(user);
    res.send('user registered successfully');
  });

  app.listen(3000, () => console.log('Server listening on http://localhost:3000'));
})();
