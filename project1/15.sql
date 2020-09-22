select owner_id, count(id) as 'Pnum'
from CatchedPokemon
group by owner_id
order by owner_id limit 1;
