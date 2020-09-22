select count(distinct p.pid)
from CatchedPokemon p, Trainer t
where p.owner_id = t.id and t.hometown = 'Sangnok City'