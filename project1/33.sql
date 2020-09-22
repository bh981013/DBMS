select sum(level)
from CatchedPokemon
where owner_id in (select id from Trainer where name = 'Matis');