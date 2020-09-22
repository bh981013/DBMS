select name
from Trainer
where id in (select owner_id
from CatchedPokemon
where level <= 10)
order by name;